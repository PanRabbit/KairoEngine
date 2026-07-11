#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform sampler2D sample1;
uniform sampler2D sample2;

void main()
{
    // apply texture
    vec4 tex1 = texture(sample1, TexCoord);
    vec4 tex2 = texture(sample2, TexCoord);

    // Simple texture layering
    if (tex2.a == 0)
        FragColor = tex1;
    else
        FragColor = tex2;


    // ---phong lighting---

    // diffuse 
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;


    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular);
    // FragColor = vec4(Normal, 1.0f);
    FragColor = FragColor * vec4(result, 1.0f);


}