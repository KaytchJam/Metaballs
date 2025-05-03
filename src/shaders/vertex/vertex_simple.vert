#version 330

in vec3 pPos;
uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(pPos, 1.0);
}