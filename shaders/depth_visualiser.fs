#version 330 core
out vec4 FragColor;

float near = 0.1; 
float far = 10; 

float LinearizeDepth(float depth)
{
    float ndc = gl_FragCoord.z * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - ndc * (far - near));
}

void main()
{
    float depth = LinearizeDepth(gl_FragCoord.z) / far;
    FragColor = vec4(vec3(1-depth), 1.0);
}