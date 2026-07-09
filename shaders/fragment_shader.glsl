#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D sample1;
uniform sampler2D sample2;

uniform vec4 col;

void main()
{
    vec4 tex1 = texture(sample1, TexCoord);
    vec4 tex2 = texture(sample2, TexCoord);

    if (tex2.a == 0)
        FragColor = tex1;
    else
        FragColor = tex2;

    FragColor = FragColor * (col + vec4(ourColor, 1.0f));
}