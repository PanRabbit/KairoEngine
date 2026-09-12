#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D image;
uniform bool horizontal;

// Gaussian kernel weights for 9-tap blur (commonly used in bloom effects).
// These specific weights correspond to a discrete approximation of a 1D Gaussian with standard deviation ~2.7.
// They sum to ~1 and represent [0, ±1, ±2, ±3, ±4] pixel offsets from the center.
// The center weight should be the largest, and the weights decrease symmetrically.
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
uniform float radius = 1.0;

void main()
{
    vec2 tex_offset = 1.0 / textureSize(image, 0); // get texel size
    vec3 result = texture(image, TexCoords).rgb * weight[0]; // center weight
    if (horizontal)
    {
        for (int i = 1; i < 5; ++i)
        {
            // blur horizontally by sampling in both positive and negative x directions and adding the samples * weights
            result += texture(image, TexCoords + vec2(tex_offset.x * i * radius, 0.0)).rgb * weight[i]; // positive offset
            result += texture(image, TexCoords - vec2(tex_offset.x * i * radius, 0.0)).rgb * weight[i]; // negative offset 
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            // blur vertically
            result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i * radius)).rgb * weight[i]; // positive offset
            result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i * radius)).rgb * weight[i]; // negative offset 
        }
    }
    FragColor = vec4(result, 1.0);
}