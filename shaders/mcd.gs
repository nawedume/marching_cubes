#version 430 core

uniform sampler3D screenTexture;

layout (points) in;
layout (triangle_strip, max_vertices = 12) out;
out vec3 outVec;

void main()
{
    outVec = texture(screenTexture, vec3(0.5)).xyz;
    EmitVertex();
    outVec = vec3(-10.0); //texture(screenTexture, vec3(0.5)).xyz;
    EmitVertex();
    outVec = texture(screenTexture, vec3(0.5)).xyz;
    EmitVertex();
    EndPrimitive();

}