#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;  // 6 faces × 3 vertices

in vec3 FragPos[];
uniform mat4 lightSpaceMatrices[6];

out vec3 FragPosGS;

void main() {
    for (int face = 0; face < 6; ++face) {
        gl_Layer = face;
        for (int i = 0; i < 3; ++i) {
            FragPosGS = FragPos[i];
            gl_Position = lightSpaceMatrices[face] * vec4(FragPos[i], 1.0);
            EmitVertex();
        }
        EndPrimitive();
    }
}
