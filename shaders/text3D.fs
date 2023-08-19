#version 430 core

in vec2 vPos;

out float voxelValue;

uniform vec3 chunkPosition;
uniform float planeZLocal;

// noise funciton in worldspace
float calculate_sphere_val(vec3 p)
{
    //return pow(0.5, 2) - pow(p.x, 2.0) - pow(p.y, 2.0) - pow(p.z, 2.0);
    return -p.y;
}

void main()
{
    // Get the z world coordinate. PlaneZLocal is already adjusted for stepsize.
    float zWorld = chunkPosition.z + planeZLocal;
    float sphere_val = calculate_sphere_val(vec3(vPos, zWorld));
    voxelValue = vPos.y;
}
