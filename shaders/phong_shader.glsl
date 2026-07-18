#version 330 core

// materials definition
struct Material {
    bool useDiffuseMap;
    sampler2D diffuseMap;
    vec3 diffuseColor;

    bool useChecker;
    float checkerSize;
    vec3 secondaryColor;

    bool useSpecularMap;
    sampler2D specularMap;
    float specularStrength;

    float ambientStrength;
    float shininess;

};
uniform Material material;
vec3 diffuseTex, specularTex;


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



out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;



// handle directional lighting
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = clamp(dot(normal, lightDir), 0.0, 1.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(clamp(dot(viewDir, reflectDir), 0.0, 1.0), material.shininess);

    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff * diffuseTex;
    vec3 specular = light.specular * spec * specularTex;
    vec3 result = ambient + diffuse + specular;
    result *= result * light.intensity;
    return (result);
}

// handle point light
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // --- PHONG LIGHTING ---
    vec3 lightDir = normalize(light.position - FragPos);
    float distance = length(light.position - FragPos);
    
    // light attenuation equation from UE4 : https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf (page 12)
    float ratio = distance / light.radius;
    float ratio4 = ratio * ratio * ratio * ratio; // manual multiplication is cheaper that doing pow(ratio, 4)
    float clamped = clamp(1.0 - ratio4, 0.0, 1.0);
    float numerator = clamped * clamped;
    float denominator = (distance * distance) + 1.0;
    float attenuation = numerator / denominator;

    
    // Ambient (Light * Texture)
    vec3 ambient = light.ambient * diffuseTex;
    
    // Diffuse (Light * Angle * Texture)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseTex;

    // Specular (Light * Reflection * Specular Map)
    viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularTex;

    // Combine light passes
    vec3 result =  (diffuse + specular + ambient) * attenuation * light.intensity;
    return result;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // check if angle is inside the spotlight cone
    float theta = dot(lightDir, normalize(-light.direction));
    // create soft border area for cone
    float epsilon = light.cutOff - light.outerCutOff;
    // calculate cone intensity
    float coneIntensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    if (coneIntensity <= 0.0) {
        return vec3(0.0);
    }
    float distance = length(light.position - FragPos);

    // light attenuation equation from UE4 : https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf (page 12)
    float ratio = distance / light.radius;
    float ratio4 = ratio * ratio * ratio * ratio; // manual multiplication is cheaper that doing pow(ratio, 4)
    float clamped = clamp(1.0 - ratio4, 0.0, 1.0);
    float numerator = clamped * clamped;
    float denominator = (distance * distance) + 1.0;
    float attenuation = numerator / denominator;

    // Ambient (Light * Texture)
    vec3 ambient = light.ambient * diffuseTex;
    
    // Diffuse (Light * Angle * Texture)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseTex;

    // Specular (Light * Reflection * Specular Map)
    viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularTex;

    // Combine light passes
    vec3 result =  (diffuse + specular + ambient) * attenuation * light.intensity * coneIntensity;

    return (result);

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


    // sample the textures/diffuse colours (converted to vec3 as lighting is calculated in vec3)
    if (material.useDiffuseMap) 
    {
        vec4 diffuseMapSample = texture(material.diffuseMap, TexCoord); 
        //lerp between the diffuse colour and the texture based on alpha value
        diffuseTex = ((1 - diffuseMapSample.a) * diffuseTex) + (diffuseMapSample.a * diffuseMapSample.rgb);
    }

    // sample spec texture
    if (material.useSpecularMap) {specularTex = vec3(texture(material.specularMap, TexCoord))* material.specularStrength;}
    else {specularTex = vec3(material.specularStrength);}
    

    // calc lighting
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // directional ligting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // point lights
    for (int i = 0; i < NR_POINT_LIGHTS; ++i)
    {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }

    // spot light
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
    
    // Output final color
    FragColor = vec4(result, 1.0);
}