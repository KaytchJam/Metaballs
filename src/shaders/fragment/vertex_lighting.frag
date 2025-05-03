#version 330

uniform vec3 lightPos;
uniform vec3 color;

in vec3 normal;
in vec4 wsPos;

out vec4 frag_color;

void main() {
    vec3 ka = vec3(0.6);
    vec3 kd = vec3(0.4);

    vec3 lightDir = lightPos - wsPos.xyz;
    float dot_nl = dot(normalize(lightDir), normalize(normal));
    dot_nl = clamp(dot_nl, 0.0, 1.0);


    frag_color = vec4(clamp(ka + kd * dot_nl, 0.0, 1.0) * color, 1.0);
}