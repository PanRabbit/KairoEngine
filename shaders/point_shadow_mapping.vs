#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;

out vec3 FragPos;  // world-space position for geo shader

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    gl_Position = worldPos;  // geo shader will transform to each face's clip space
}