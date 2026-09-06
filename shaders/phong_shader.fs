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
#define MAX_POINT_LIGHTS 8
uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform samplerCube pointShadowMaps[MAX_POINT_LIGHTS];
uniform float pointLightFarPlane;

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

uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;

uniform float scrWidth;
uniform float scrHeight;

vec2 screenCoord = gl_FragCoord.xy / vec2(scrWidth, scrHeight);

uniform bool isSelected = false;

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;



float CalcDirShadows(vec4 FragPosLightSpace)
{
    // Shadow maping with PCF (Percentage Closer Filtering)
    float shadow = 0.0;
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w; // normalized device coordinates [-1, 1] (pointless for orthographic projection, but good practice)
    projCoords = projCoords * 0.5 + 0.5; // transform to [0, 1] range
    float currentDepth = projCoords.z; // depth value of current fragment
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float bias = 0.0005;

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            if (currentDepth > 1.0) shadow = 0.0; // if the current fragment is behind the light, it is not in shadow
        }
    }
    shadow /= 9.0;
    return shadow;
}

float CalcPointShadow(PointLight light, samplerCube shadowMap, vec3 fragPos)
{

vec3 sampleOffsetDirections[20] = vec3[]
    (
        vec3( 1, 1, 1), vec3( 1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
        vec3( 1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
        vec3( 1, 1, 0), vec3( 1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
        vec3( 1, 0, 1), vec3(-1, 0, 1), vec3( 1, 0, -1), vec3(-1, 0, -1),
        vec3( 0, 1, 1), vec3( 0, -1, 1), vec3( 0, -1, -1), vec3( 0, 1, -1)
    );

    vec3 lightToFrag = fragPos - light.position;
    float currentDepth = length(lightToFrag) / pointLightFarPlane;
    if (currentDepth > 1.0)
        return 0.0;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float bias = 0.0005;
    int samples = 20;
    float discRadius = 0.01;

    for (int i = 0; i < samples; ++i) {
        float pcfDepth = texture(shadowMap, lightToFrag + sampleOffsetDirections[i] * discRadius).r;
        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        if (currentDepth > 1.0) shadow = 0.0; // if the current fragment is behind the light, it is not in shadow
    }


    shadow /= float(samples);
    return shadow;
}



// handle directional lighting
void CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, out vec3 outDiffuse, out vec3 outSpecular, out vec3 outAmbient)
{
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir); // blinn-phong

    float diff = clamp(dot(normal, lightDir), 0.0, 1.0);
    float spec = pow(clamp(dot(normal, halfwayDir), 0.0, 1.0), material.shininess); // blinn-phong

    vec3 ambient = light.ambient * diffuseTex;
    vec3 diffuse = light.diffuse * diff * diffuseTex;
    
    outDiffuse = diffuse * light.intensity;
    outSpecular = (light.specular * spec * specularTex) * light.intensity;
    outAmbient = ambient * light.intensity;
}

// handle point light
void CalcPointLight(PointLight light, samplerCube shadowMap, vec3 normal, vec3 fragPos, vec3 viewDir, out vec3 outDiffuse, out vec3 outSpecular, out vec3 outAmbient)
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

    outDiffuse = diffuse * attenuation * light.intensity;
    outSpecular = (light.specular * spec * specularTex) * attenuation * light.intensity;
    outAmbient = ambient * attenuation * light.intensity;

    // calc shadows
    float shadow = CalcPointShadow(light, shadowMap, fragPos);

    outDiffuse *= (1.0 - shadow);
    outSpecular *= (1.0 - shadow);
    // out ambient is not affected by shadow

}

// handle spot light
void CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, out vec3 outDiffuse, out vec3 outSpecular, out vec3 outAmbient)
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

    outDiffuse = diffuse * attenuation * light.intensity * coneIntensity;
    outSpecular = (light.specular * spec * specularTex) * attenuation * light.intensity * coneIntensity;
    outAmbient = ambient * attenuation * light.intensity * coneIntensity;
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

    // Accumulators for diffuse and specular and ambient
    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);
    vec3 totalAmbient = vec3(0.0);
    vec3 d, s, a;

    // Directional lighting
    CalcDirLight(dirLight, norm, viewDir, d, s, a);
    float shadow = CalcDirShadows(FragPosLightSpace); // sun shadow
    totalDiffuse += d * (1.0 - shadow);
    totalSpecular += s * (1.0 - shadow);
    totalAmbient += a;

    // Point lights (numPointLights is set from the level; cap is MAX_POINT_LIGHTS)
    int pointCount = min(numPointLights, MAX_POINT_LIGHTS);
    for (int i = 0; i < pointCount; ++i)
    {
        CalcPointLight(pointLights[i], pointShadowMaps[i], norm, FragPos, viewDir, d, s, a);
        totalDiffuse += d;
        totalSpecular += s;
        totalAmbient += a;
    }

    // Spot lights
    CalcSpotLight(spotLight, norm, FragPos, viewDir, d, s, a);
    totalDiffuse += d;
    totalSpecular += s;
    totalAmbient += a;


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
    result = result + totalAmbient;

    if (isSelected) {result = result / (1.0 - vec3(0.2, 0.55, 0.85)); } // color dodge

    FragColor = vec4(result, alphaTex);
}