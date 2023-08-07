#version 430 core

in vec2 vPos;

out float voxelValue;

uniform vec3 chunkPosition;
uniform float planeZLocal;

// noise funciton in worldspace
float calculate_sphere_val(vec3 coord)
{
    return pow(0.9, 2) - pow(coord.x, 2.0) - pow(coord.y, 2.0) - pow(coord.z, 2.0);
}

void main()
{
    float zWorld = chunkPosition.z + planeZLocal;
    float sphere_val = calculate_sphere_val(vec3(vPos, zWorld));
    voxelValue = sphere_val;
}
