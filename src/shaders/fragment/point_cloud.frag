#version 330

in float weight;
out vec4 fragment;

void main() {
    fragment = vec4(vec3(1.0) * weight, 1.0);
}