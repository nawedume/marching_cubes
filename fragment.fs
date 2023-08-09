#version 330 core

in vec3 vWorldPos;
in vec3 Norms;
in vec2 textcoord;

uniform vec3 viewPos;
uniform sampler2D ourTexture;

out vec4 FragColor;

void main() {
	vec3 color = vec3(1.0,1.0,1.0);
	vec3 lightDir = normalize(vec3(0.0,-1.0,-1.0));
	vec3 lightColor = vec3(1.0,1.0,1.0);

	vec3 norm = normalize(Norms);
	float diff = max(dot(norm,lightDir), 0.0);
	vec3 ds = diff * lightColor;

	float ambientStrength = 0.1;
	vec3 ambience = ambientStrength*lightColor;

	// specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - vWorldPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  

	color = color * (ds + ambience + specular);
	FragColor = texture(ourTexture, textcoord) * clamp(vec4(color, 1.0),0.0, 1.0);
}
