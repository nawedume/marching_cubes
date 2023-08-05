#version 330 core

layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;

out vec3 vWorldPos;
out vec3 vNormal;

uniform mat4 projection;
uniform mat4 view;

void main() {
	gl_Position = projection * view * vec4(aPos,1.0);
	vWorldPos = aPos;
	vNormal = aNormal;
}
