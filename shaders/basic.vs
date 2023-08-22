#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

out vec3 vPos;
out vec3 vNormal;

uniform mat4 uWorldTransform;
uniform mat4 uCameraTransform;
uniform mat4 uPerspectiveTransform;

void main()
{
  vec4 worldPos = uWorldTransform * vec4(inPosition, 1.0); 
  gl_Position = uPerspectiveTransform * uCameraTransform * worldPos;
  vPos = worldPos.xyz;
  vNormal = inNormal;
}
