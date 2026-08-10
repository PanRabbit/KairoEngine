#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform vec3 Color;

void main()
{   
    FragColor = vec4(Color, 1.0f); // Just pass through the color from the vertex shader
}