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
	//vec3 lighting = BlinPhong(viewPos, normalize(Norms), vWorldPos, lightPos, vec3(1.0));
	//color *= lighting;
	//color = pow(color, vec3(1.0/2.2));
    vec3 ambient = 0.05 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - vWorldPos);
	vec3 normal = normalize(Norms);
	vec3 viewDir = normalize(viewPos - vWorldPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color; 
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 2.0);
	vec3 specular = vec3(0.3) * spec; // assuming bright white light color
	FragColor = vec4(ambient + diffuse + specular, 1.0);
}
