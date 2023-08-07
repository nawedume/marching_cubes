#version 430 core

// position of the quad
layout (location = 0) in vec2 inPos; 

out vec2 vPos;

// Indicates the (0, 0, 0) position of the chunk in world space.
uniform vec3 chunkPosition;
uniform float numVoxelPerDim;
uniform float voxelSize;

void main()
{
    gl_Position = vec4(inPos, 0.0, 1.0);
    
    // Converts the Quad coordinates to world position coordinates.
    vPos = (((((inPos + 1.0) / 2.0) * (numVoxelPerDim + 1))) * voxelSize) + chunkPosition.xy;
}
