#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform vec4 col;

void main()
{   
    float scale = 0.1f;
    float step = floor(TexCoord.x / scale) + floor(TexCoord.y / scale);
    float mod = mod(step, 2);
    FragColor = (mod + vec4(ourColor, 1.0f));
    if (FragColor.r > 0.6)
       FragColor = col / 2;
       else
            FragColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);
}