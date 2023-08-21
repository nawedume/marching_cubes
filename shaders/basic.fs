#version 330 core

const vec3 UP = vec3(0.0, 1.0, 0.0);

in vec3 vNormal;

out vec4 outColor;

uniform vec3 uLightDir;

void main()
{
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(uLightDir);
    float diffuseStrength = max(0.0, dot(normal, lightDir));
    const float ambientStrength = 0.3;

    float tilt = dot(normal, UP);
    vec3 color = tilt < 0.5 ? vec3(0.4196, 0.4235, 0.4157) : vec3(0.0, 0.7, 0.1) ;

    
    outColor = clamp(vec4(color * (ambientStrength + diffuseStrength), 1.0), 0.0, 1.0);
}
