#version 330 core

layout (location=0) in vec3 aPos;
layout (location=1) in vec3 anorms;
layout (location=2) in vec2 aText;

out vec3 vWorldPos;
out vec3 Norms;
out vec2 textcoord;

uniform mat4 projection;
uniform mat4 view;

void main() {
	gl_Position = projection * view * vec4(aPos,1.0);
	vWorldPos = aPos;
	Norms = anorms;
	textcoord = vec2(aText.x, aText.y);
}
