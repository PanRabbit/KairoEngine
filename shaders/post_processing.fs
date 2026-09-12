#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomBlur;
uniform float time;
uniform float scrWidth;
uniform float scrHeight;
uniform float exposure;

uniform bool enableSharpen;
uniform float sharpness;

uniform bool enableBlur;
uniform float blurStrength;

uniform bool enableEdgeDetection;
uniform float edgeDetectionStrength;

uniform bool enablePixelate;
uniform float pixelateResolution;

uniform bool enableBloom;
uniform float bloomIntensity;

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

    vec2[9] sharpOffsets = getOffsets(sharpness / max(scrHeight, 1.0));

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

    vec2[9] edgeDetectionOffsets = getOffsets(edgeDetectionStrength / max(scrHeight, 1.0));

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

    vec2[9] blurOffsets = getOffsets(blurStrength / max(scrHeight, 1.0));

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
    float pixelSize = scrHeight / max(resolution, 1.0);
    float pixelsX = scrWidth / pixelSize;
    float pixelsY = scrHeight / pixelSize;
    vec2 NewTexCoords = vec2((floor(TexCoords.x * (pixelsX))) / (pixelsX), (floor(TexCoords.y * (pixelsY))) / (pixelsY));
    return NewTexCoords;
}

void main()
{
    vec2 uv = TexCoords;
    if (enablePixelate)
        uv = pixelate(pixelateResolution);

    vec3 original = texture(screenTexture, uv).rgb;
    vec3 color = original;

    // Stack effects
    if (enableBlur)
        color = blur(uv);
    if (enableSharpen)
        color += sharpen(uv) - original;
    if (enableEdgeDetection)
        color += edgeDetection(uv);
    if (enableBloom)
        color += texture(bloomBlur, TexCoords).rgb * bloomIntensity;

    color *= exposure;
    FragColor = vec4(color, 1.0);
}
