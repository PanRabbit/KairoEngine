#version 330 core
in vec3 FragPosGS;
uniform vec3 lightPos;
uniform float farPlane;

void main() {
    gl_FragDepth = length(FragPosGS - lightPos) / farPlane;
}
