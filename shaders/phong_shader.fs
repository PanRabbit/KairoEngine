#version 330 core

// materials definition
struct Material {
    bool useDiffuseMap;
    sampler2D diffuseMap;
    vec3 diffuseColor;

    bool useAlphaMap;
    sampler2D alphaMap;

    bool useChecker;
    float checkerSize;
    vec3 secondaryColor;

    bool useSpecularMap;
    sampler2D specularMap;
    float specularStrength;

    float shininess;

    vec2 coordOffset;
    vec2 coordScale;
};
uniform Material material;
vec3 diffuseTex, specularTex;
float alphaTex;


//lighting defs
struct DirLight {
    vec3 direction;

    float intensity;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

struct PointLight {
    vec3 position;

    float radius;
    float intensity;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

struct SpotLight {
    vec3 position;
    vec3 direction;
    
    float cutOff;
    float outerCutOff;

    float radius;
    float intensity;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform SpotLight spotLight;

uniform samplerCube skybox;
uniform vec3 viewPos;

uniform float scrWidth;
uniform float scrHeight;

vec2 screenCoord = gl_FragCoord.xy / vec2(scrWidth, scrHeight);

uniform bool isSelected = false;

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;


// handle directional lighting
void CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, out vec3 outDiffuse, out vec3 outSpecular)
{
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir); // blinn-phong

    float diff = clamp(dot(normal, lightDir), 0.0, 1.0);
    float spec = pow(clamp(dot(normal, halfwayDir), 0.0, 1.0), material.shininess); // blinn-phong

    vec3 ambient = light.ambient * diffuseTex;
    vec3 diffuse = light.diffuse * diff * diffuseTex;
    
    outDiffuse = (ambient + diffuse) * light.intensity;
    outSpecular = (light.specular * spec * specularTex) * light.intensity;
}

// handle point light
void CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, out vec3 outDiffuse, out vec3 outSpecular)
{
    // --- PHONG LIGHTING ---
    vec3 lightDir = normalize(light.position - FragPos);
    float distance = length(light.position - FragPos);
    
    // light attenuation equation from UE4 : https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf (page 12)
    float ratio = distance / light.radius;
    float ratio4 = ratio * ratio * ratio * ratio; // manual multiplication is cheaper (?) that doing pow(ratio, 4)
    float clamped = clamp(1.0 - ratio4, 0.0, 1.0);
    float numerator = clamped * clamped;
    float denominator = (distance * distance) + 1.0;
    float attenuation = numerator / denominator;

    // Ambient + Diffuse (Light * Texture)
    vec3 ambient = light.ambient * diffuseTex;
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseTex;

    // Specular (Light * Reflection * Specular Map)
    vec3 halfwayDir = normalize(lightDir + viewDir); // blinn-phong
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess); // blinn-phong

    outDiffuse = (ambient + diffuse) * attenuation * light.intensity;
    outSpecular = (light.specular * spec * specularTex) * attenuation * light.intensity;
}

// handle spot light
void CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, out vec3 outDiffuse, out vec3 outSpecular)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // check if angle is inside the spotlight cone
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float coneIntensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    if (coneIntensity <= 0.0) {
        outDiffuse = vec3(0.0);
        outSpecular = vec3(0.0);
        return;
    }
    float distance = length(light.position - FragPos);

    // light attenuation equation from UE4 : https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf (page 12)
    float ratio = distance / light.radius;
    float ratio4 = ratio * ratio * ratio * ratio; // manual multiplication is cheaper (?) that doing pow(ratio, 4)
    float clamped = clamp(1.0 - ratio4, 0.0, 1.0);
    float numerator = clamped * clamped;
    float denominator = (distance * distance) + 1.0;
    float attenuation = numerator / denominator;

    // Ambient + Diffuse
    vec3 ambient = light.ambient * diffuseTex;
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseTex;

    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir); // blinn-phong
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess); // blinn-phong

    outDiffuse = (ambient + diffuse) * attenuation * light.intensity * coneIntensity;
    outSpecular = (light.specular * spec * specularTex) * attenuation * light.intensity * coneIntensity;
}


void main()
{
    // handle map options from material definition
    if (material.useChecker) {
        //checker pattern
        float step = floor(TexCoord.x / material.checkerSize) + floor(TexCoord.y / material.checkerSize);
        float mod = mod(step, 2);
        diffuseTex = (mod + vec3(0.5f));
        if (diffuseTex.r > 0.6) diffuseTex = material.diffuseColor;
        else diffuseTex = material.secondaryColor;
    }
    else diffuseTex = material.diffuseColor;

    // sample alpha texture
    if (material.useAlphaMap) { alphaTex = texture(material.alphaMap, TexCoord * material.coordScale + material.coordOffset).r; }
    else { alphaTex = 1.0; }

    if (alphaTex < 0.01) discard;

    // sample the textures/diffuse colours
    if (material.useDiffuseMap) 
    {
        vec4 diffuseMapSample = texture(material.diffuseMap, TexCoord * material.coordScale + material.coordOffset); 
        diffuseTex = ((1.0 - diffuseMapSample.a) * diffuseTex) + (diffuseMapSample.a * diffuseMapSample.rgb);
        alphaTex = min(alphaTex, diffuseMapSample.a);
    }

    // sample spec texture
    if (material.useSpecularMap) { specularTex = vec3(texture(material.specularMap, TexCoord * material.coordScale + material.coordOffset)) * material.specularStrength; }
    else { specularTex = vec3(material.specularStrength); }

    // calc lighting geometry
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Accumulators for diffuse and specular
    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);
    vec3 d, s;

    // Directional lighting
    CalcDirLight(dirLight, norm, viewDir, d, s);
    totalDiffuse += d;
    totalSpecular += s;

    // Point lights
    for (int i = 0; i < NR_POINT_LIGHTS; ++i)
    {
        CalcPointLight(pointLights[i], norm, FragPos, viewDir, d, s);
        totalDiffuse += d;
        totalSpecular += s;
    }

    // Spot lights
    CalcSpotLight(spotLight, norm, FragPos, viewDir, d, s);
    totalDiffuse += d;
    totalSpecular += s;


    // Skybox reflection sample
    vec3 I = normalize(FragPos - viewPos);
    vec3 R = reflect(I, norm);

    float roughness = sqrt(2.0 / (material.shininess + 2.0));
    roughness = clamp(roughness / max(specularTex.r, 0.001), 0.0, 1.0);

    float maxLod   = 9.0; 
    float lod = roughness * maxLod;
    vec3 reflection = textureLod(skybox, R, lod).rgb;

    // Apply reflection with realistic energy conservation + fresnel

    // view angle
    float NdotV = max(dot(norm, viewDir), 0.0);

    // base reflectance
    vec3 F0 = specularTex * 0.33; // multiplied down as a placeholder for metallicness

    vec3 kS = F0 + (vec3(1.0) - F0) * pow(1.0 - NdotV, 5.0);     // % of light reflected
    vec3 kD = vec3(1.0) - kS;        // Remaining energy available for diffuse

    // combine diffuse, specular & skybox reflection
    vec3 result = (totalDiffuse * kD) + totalSpecular + (reflection * kS);

    if (isSelected) { result = result / (1.0 - vec3(0.2, 0.55, 0.85)); } // color dodge

    FragColor = vec4(result, alphaTex);
}