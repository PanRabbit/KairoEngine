#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{   

    //checker pattern
    float scale = 0.1f;
    float step = floor(TexCoord.x / scale) + floor(TexCoord.y / scale);
    float mod = mod(step, 2);
    FragColor = (mod + vec4(0.5f, 0.5f, 0.5f, 1.0f));
    if (FragColor.r > 0.6)
       FragColor = vec4(1.0f, 0.1f, 0.1f, 1.0f);
       else
            FragColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);



   // ---phong lighting---

    // diffuse 
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;


    // specular
    float specularStrength = 2.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular);
    // FragColor = vec4(Normal, 1.0f);
    FragColor = FragColor * vec4(result, 1.0f);
}