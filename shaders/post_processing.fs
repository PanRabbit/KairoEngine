#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;

void main()
{
    vec2 NewTexCoords = vec2(TexCoords.x, 1.0 - TexCoords.y);
    FragColor = vec4(vec3(1.0 - texture(screenTexture, NewTexCoords)), 1.0);
}