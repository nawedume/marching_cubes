#version 330 core

const vec3 UP = vec3(0.0, 1.0, 0.0);

in vec3 vPos;
in vec3 vNormal;

out vec4 outColor;

uniform vec3 uLightDir;

uniform sampler2D rockTexture;
uniform sampler2D groundTexture;

vec4 get_texture_color(sampler2D text, vec3 coords, vec3 blend)
{
  vec4 xaxis = texture(text, coords.yz);
  vec4 yaxis = texture(text, coords.xz);
  vec4 zaxis = texture(text, coords.xy);

  vec4 final = xaxis*blend.x + yaxis*blend.y + zaxis*blend.z;
  return final;

};

void main()
{
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(uLightDir);
    float diffuseStrength = max(0.0, dot(normal, lightDir));
    const float ambientStrength = 0.3;

    // calculate the blend factor
    vec3 blending = abs(normal);
    blending = normalize(max(blending, 1e-5));
    float b = (blending.x + blending.y + blending.z);
    blending /= vec3(b);

    float tilt = dot(normal, UP);
    //vec3 textureColor = abs(normal.x)*rock1 + abs(normal.z)*rock2 + abs(normal.y)*ground;
    vec4 ground = get_texture_color(groundTexture, vPos / 100.0, blending);
    vec4 rock = get_texture_color(rockTexture, vPos / 100.0, blending);
    vec4 textureColor = tilt > 0.6 ? ground : rock;

    vec3 color = textureColor.xyz;
    outColor = clamp(vec4(color * (ambientStrength + diffuseStrength), 1.0), 0.0, 1.0);
}
