#version 430 core

in vec2 vTexCoord;

out vec4 fColor;

uniform sampler3D screenTexture;

uniform int layerIndex;

void main()
{
    vec3 coord = vec3(vTexCoord, float(layerIndex) / 33.0);
    fColor.xyz = texture(screenTexture, coord).xyz;
    fColor.w = 1.0;
}