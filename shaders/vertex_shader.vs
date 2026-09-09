#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 aTangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform mat4 transform;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec4 FragPosLightSpace;
out mat3 TBN;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoord = aTexCoord;
    FragPosLightSpace = lightSpaceMatrix * model * vec4(aPos, 1.0);
    
    // calculate TBN matrix (tangent-bitangent-normal)
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent.xyz);
    T = normalize(T - N * dot(N, T));
    vec3 B = cross(N, T) * aTangent.w;
    Normal = N;
    TBN = mat3(T, B, N);
}