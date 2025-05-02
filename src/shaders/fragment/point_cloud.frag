#version 330

in float weight;
out vec4 fragment;

void main() {
    fragment = vec4(vec3(weight), 1.0);
}