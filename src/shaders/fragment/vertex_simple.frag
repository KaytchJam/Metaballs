#version 330

out vec4 frag_color;

void main() {
    vec3 color = vec3(1.0);
    frag_color = vec4(color, 1.0);
}