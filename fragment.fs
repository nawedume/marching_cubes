#version 330 core

in vec3 vWorldPos;
in vec3 Norms;
in vec2 textcoord;

uniform vec3 viewPos;
uniform sampler2D ourTexture;
uniform vec3 lightPos;
out vec4 FragColor;


vec3 BlinPhong(vec3 viewPos, vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor)
{
	//diffuse
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff*lightColor;

	// specular
	vec3 viewDir = normalize(viewPos - fragPos);
	float spec = 0.0;
	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	vec3 specular = spec * lightColor;

	// simple attenuation
	float distance = length(lightPos - fragPos);
	float attenuation = 1.0 / (distance);

	diffuse *= attenuation;
	specular *= attenuation;

	return diffuse + specular;  
}

void main() {
	vec3 color = texture(ourTexture, textcoord).rgb;
	vec3 lightColor = vec3(1.0f);

	float ambientStrength = 0.5f;
    vec3 ambient = ambientStrength * lightColor;

	vec3 norm = normalize(Norms);
	vec3 lightDir = normalize(lightPos - vWorldPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * lightColor;

	

	vec3 viewDir = normalize(viewPos - vWorldPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	float shininess = 64.0f;
	float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
	vec3 specular = lightColor * spec * color;

    vec3 result = (ambient + diffuse + specular) * color;
    FragColor = vec4(result, 1.0);


	
}
