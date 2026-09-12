#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float threshold;

void main()
{
    vec3 color = texture(screenTexture, TexCoords).rgb;

    // Calculate the perceived brightness of the color using standard luminance coefficients for RGB.
    // These coefficients (0.2126, 0.7152, 0.0722) represent the human eye's sensitivity to red, green, and blue, respectively,
    // according to the Rec. 709 standard. The dot product applies this weighting to get a single brightness value.
    //float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

    float brightness = max(color.r, max(color.g, color.b)); // alternative to catch colors instead of luminance


    FragColor = vec4(brightness > threshold ? color : vec3(0.0), 1.0);
}