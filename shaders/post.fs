#version 430 core

in vec2 vTexCoord;

out vec4 fColor;

uniform sampler3D screenTexture;

uniform int layerIndex;

void main()
{
    fColor.xyz = texture(screenTexture, vec3(vTexCoord, float(layerIndex) / 33.0)).xyz;
    fColor.w = 1.0;
}