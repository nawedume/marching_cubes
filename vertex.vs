#version 330 core

layout (location=0) in vec3 aPos;
layout (location=1) in vec3 anorms;
layout (location=2) in vec2 aText;
layout (location=3) in vec3 aTangent;

out vec3 vWorldPos;
out vec2 textcoord;
out vec3 TviewPos;
out vec3 TlightDir;
out mat3 TBN;
out vec3 Norms;


uniform mat4 projection;
uniform mat4 view;
uniform vec3 lightDir;
uniform vec3 viewPos;

void main() {

	vec3 T = normalize(aTangent);
	vec3 N = normalize(anorms);
	vec3 B = cross(N,T);

	TBN = transpose(mat3(T,B,N));

	vWorldPos = aPos;
	
	TlightDir = TBN*lightDir;
	TviewPos = TBN*viewPos;

	textcoord = vec2(aText.x, aText.y);

	Norms = anorms;

	gl_Position = projection * view * vec4(aPos,1.0);
}
