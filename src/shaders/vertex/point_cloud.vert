#version 330

in vec4 pPos;
uniform mat4 MVP;

out float weight;

void main() {
    weight = pPos.w;
    gl_Position = MVP * vec4(pPos.xyz, 1.0);
}