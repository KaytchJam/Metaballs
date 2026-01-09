#version 330

uniform mat4 MVP;

in vec3 pPos;
in vec3 pNorm;

out vec3 normal;
out vec4 wsPos;

void main() {
    normal = pNorm;
    wsPos = vec4(pPos, 1.0);
    gl_Position = MVP * vec4(pPos, 1.0);
}