#version 330 core

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

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light;

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;

void main()
{
    vec3 diffuseTex, specularTex;

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
    
    // --- PHONG LIGHTING ---
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    
    // Ambient (Light * Texture)
    vec3 ambient = light.ambient * material.ambientStrength * diffuseTex;
    
    // Diffuse (Light * Angle * Texture)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseTex;

    // Specular (Light * Reflection * Specular Map)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularTex;

    // 2. Combine all three light phases
    vec3 result = ambient + diffuse + specular;
    
    // 3. Output final color
    FragColor = vec4(result, 1.0);
}