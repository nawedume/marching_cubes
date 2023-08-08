#version 330 core

in vec3 vWorldPos;
in vec3 Norms;

out vec4 FragColor;


void main() {
	float diff = max(dot(Norms,vec3(0.0,2.0,-2.0)), 0.0);
	vec3 ds = diff * vec3(1.0);
	float ambientStrength = 0.5;
	vec3 ambience = ambientStrength*vec3(1.0);
	vec3 color = vec3(1.0,1.0,1.0);
	color = color * (ds + ambience);
	FragColor = clamp(vec4(color, 1.0),0.0, 1.0);
}
