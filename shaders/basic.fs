#version 330 core

in vec3 vNormal;

out vec4 outColor;

uniform vec3 uLightDir;

void main()
{
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(uLightDir);

    float diffuseStrength = max(0.0, dot(normal, lightDir));
    const float ambientStrength = 0.5;
    vec3 color = vec3(0.6, 0.6, 0.6) * (ambientStrength + diffuseStrength);
    outColor = clamp(vec4(color, 1.0), 0.0, 1.0);
}