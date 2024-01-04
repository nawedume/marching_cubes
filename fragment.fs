#version 330 core

in vec3 vWorldPos;
in vec3 Norms;
in vec2 textcoord;
in mat3 TBN;

uniform vec3 viewPos;
uniform sampler2D ourTexture;
uniform sampler2D ourTexture2;
uniform vec3 lightDir;
out vec4 FragColor;


void main() {
	vec3 colorX = texture(ourTexture, vWorldPos.yz).rgb;
	vec3 colorY = texture(ourTexture, vWorldPos.xz).rgb;
	vec3 colorZ = texture(ourTexture, vWorldPos.xy).rgb;

	vec3 blending = abs(Norms);
	blending = normalize(max(blending, 0.00001));
	float b = blending.x + blending.y + blending.z;
	blending /= vec3(b,b,b);

	vec3 color = colorX * blending.x + colorY * blending.y + colorZ + blending.z;
	vec3 lightColor = vec3(1.0f);

	float ambientStrength = 0.5f;
    vec3 ambient = ambientStrength * lightColor;

	vec3 normX = texture(ourTexture2, vWorldPos.yz).rgb;
	normX = normalize(normX * 2.0 - 1.0);
	vec3 normY = texture(ourTexture2, vWorldPos.xz).rgb;
	normY = normalize(normY * 2.0 - 1.0);
	vec3 normZ = texture(ourTexture2, vWorldPos.zy).rgb;
	normZ = normalize(normZ * 2.0 - 1.0);

	vec3 norm = normX * blending.x + normY * blending.y + normZ * blending.z;
	norm = normalize(norm);
	vec3 NlightDir = normalize(lightDir);
	float diff = max(dot(norm, NlightDir), 0.0f);
	vec3 diffuse = diff * lightColor;

	

	vec3 viewDir = normalize(viewPos - vWorldPos);
	vec3 halfwayDir = normalize(NlightDir + viewDir);

	float shininess = 1.0f;
	float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
	vec3 specular = lightColor * spec * color;

    vec3 result = (ambient + diffuse + specular) * color;
    FragColor = vec4(result, 1.0);


	
}
