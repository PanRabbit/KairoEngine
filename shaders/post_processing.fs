#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
uniform float scrWidth;
uniform float scrHeight;



vec2 pixelate()
{
    float resolution = 64.0;
    float pixelSize = scrHeight / resolution; // calc size of pixel based on resolution
    float offset = 0.025; // small offset to fix wrapping issue (lazy method cause I cba to change texture filtering mode)

    float pixelsX = scrWidth / pixelSize;
    float pixelsY = scrHeight / pixelSize;
    vec2 NewTexCoords = vec2((floor(TexCoords.x * (pixelsX))+ offset) / (pixelsX), (floor(TexCoords.y * (pixelsY)) + offset) / (pixelsY));
    return NewTexCoords;
}

void main()
{
    vec3 color = vec3(texture(screenTexture, 1.0 - TexCoords));
    vec3 newColor = pow(vec3(1.0,0.0,0.0) - color, vec3(4.2));
    //FragColor = vec4(vec3(texture(screenTexture, pixelate())), 1.0);
    FragColor = vec4(newColor, 1.0);
}