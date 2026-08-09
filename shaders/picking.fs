#version 330 core
out vec4 FragColor;

uniform int objectID; // Pass integer ID (1, 2, 3...)

void main() {
    // Convert integer ID into RGB normalized floats (base 256)
    int r = (objectID & 0x000000FF);
    int g = (objectID & 0x0000FF00) >> 8;
    int b = (objectID & 0x00FF00) >> 16;

    FragColor = vec4(float(r) / 255.0, float(g) / 255.0, float(b) / 255.0, 1.0);
}