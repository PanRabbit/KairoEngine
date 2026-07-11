#version 330 core

struct Material {
    float ambientStrength;
    sampler2D diffuseColor;
    float specularStrength;  
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


uniform int textureType; // 0 = Checker, 1 = Textured

// Textures
uniform sampler2D sample1;
uniform sampler2D sample2;

// 1. Helper function to get the base color
vec4 getBaseColor() 
{
    if (textureType == 0) 
    {
        // Checker pattern logic
        float scale = 0.1f;
        float step = floor(TexCoord.x / scale) + floor(TexCoord.y / scale);
        float checkerVal = mod(step, 2.0); // Renamed from 'mod' to avoid keyword conflicts
        
        vec4 color = checkerVal + vec4(0.5f, 0.5f, 0.5f, 1.0f);
        if (color.r > 0.6) return vec4(0.7f, 0.7f, 0.7f, 1.0f);
        else return vec4(0.1f, 0.1f, 0.1f, 1.0f);
    } 
    else if (textureType == 1) 
    {
        // Texture logic
        vec4 tex1 = texture(sample1, TexCoord);
        vec4 tex2 = texture(sample2, TexCoord);
        if (tex2.a == 0) return tex1;
        else return tex2;
    }
    
    return vec4(1.0); // Default fallback
}

void main()
{
    vec3 baseColor = getBaseColor().rgb;

    // --- PHONG LIGHTING  ---
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    
    // Diffuse & Ambient
    vec3 ambient = material.ambientStrength * light.ambient;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = material.specularStrength * spec * light.specular;


    vec3 result = (ambient + diffuse + specular);
    FragColor = getBaseColor() * vec4(result, 1.0f);
}