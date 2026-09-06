#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
uniform float scrWidth;
uniform float scrHeight;

float sharpness = 0.005;
float blurStrength = 0.25;
float edgeDetectionStrength = 0.1;

vec2[9] getOffsets(float offsetDistance)
{
    return vec2[9](
        vec2(-offsetDistance,  offsetDistance), // top-left
        vec2( 0.0f,           offsetDistance), // top-center
        vec2( offsetDistance,  offsetDistance), // top-right
        vec2(-offsetDistance,  0.0f),          // center-left
        vec2( 0.0f,            0.0f),          // center-center
        vec2( offsetDistance,  0.0f),          // center-right
        vec2(-offsetDistance, -offsetDistance), // bottom-left
        vec2( 0.0f,          -offsetDistance), // bottom-center
        vec2( offsetDistance, -offsetDistance)  // bottom-right
    );
}

vec3 sharpen(vec2 sharpCoords)
{  
    float sharpKernel[9] = float[](
        -1, -1, -1,
        -1, 9, -1,
        -1, -1, -1
    );

    vec2[9] sharpOffsets = getOffsets(sharpness / 100.0);

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, sharpCoords + sharpOffsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
    {
        col += sampleTex[i] * sharpKernel[i];
    }
    return col;
}

vec3 edgeDetection(vec2 edgeDetectionCoords)
{  
    float edgeDetectionKernel[9] = float[](
        1, 1, 1,
        1, -8, 1,
        1, 1, 1
    );

    vec2[9] edgeDetectionOffsets = getOffsets(edgeDetectionStrength / 100.0);

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, edgeDetectionCoords + edgeDetectionOffsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
    {
        col += sampleTex[i] * edgeDetectionKernel[i];
    }
    return col;
}


vec3 blur(vec2 blurCoords)
{  
    float blurKernel[9] = float[](
        1.0, 2.0, 1.0,
        2.0, 4.0, 2.0,
        1.0, 2.0, 1.0
    );

    vec2[9] blurOffsets = getOffsets(blurStrength / 100.0);

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, blurCoords + blurOffsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
    {
        col += sampleTex[i] * blurKernel[i] / 16.0;
    }
    return col;
}

vec2 pixelate(float resolution)
{
    float pixelSize = scrHeight / resolution; // calc size of pixel based on resolution
    float pixelsX = scrWidth / pixelSize;
    float pixelsY = scrHeight / pixelSize;
    vec2 NewTexCoords = vec2((floor(TexCoords.x * (pixelsX))) / (pixelsX), (floor(TexCoords.y * (pixelsY))) / (pixelsY));
    return NewTexCoords;
}

void main()
{
    vec3 color = vec3(texture(screenTexture, TexCoords));
    vec4 sharpened = vec4(sharpen(TexCoords), 1.0);
    FragColor = vec4(color, 1.0);
}