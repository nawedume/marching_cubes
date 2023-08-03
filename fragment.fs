#version 330 core

in vec3 vWorldPos;

out vec4 FragColor;

void main() {

	float ds = 0.0;
	float ambientStrength = 1.0;
	vec3 color = vec3(0.7, 0.0, 0.0) * (ds + ambientStrength);

	if (vWorldPos.y > 1.0)
	{
		color = vec3(0.98, 0.98, 0.98);
	} else if (vWorldPos.y > 0.2)
	{
		color = vec3(0.36, 0.25, 0.24);
	} else {
		color = vec3(0.1, 0.8, 0.2);
	}

	FragColor = vec4(color, 1.0);
}
