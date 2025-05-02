#version 330
 
in vec3 VFragColor;
 
out vec4 FragColor;
 
void main() {
    FragColor = vec4(VFragColor, 1.0);
}