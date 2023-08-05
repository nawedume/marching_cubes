#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;

out vec4 FragColor;

void main() {
	vec3 normal = -normalize(vNormal);	
	vec3 lightDir = normalize(vec3(0.0, 1.0, 1.0));
	float diffuseStrength = dot(normal, lightDir);
	float ambientStrength = 0.1;

	vec3 color;
	if (vWorldPos.y > 1.0)
	{
		color = vec3(0.98, 0.98, 0.98);
	} else if (vWorldPos.y > 0.2)
	{
		color = vec3(0.36, 0.25, 0.24);
	} else {
		color = vec3(0.1, 0.8, 0.2);
	}
	color = color * (diffuseStrength + ambientStrength);

	FragColor = clamp(vec4(color, 1.0), 0.0, 1.0);
}
