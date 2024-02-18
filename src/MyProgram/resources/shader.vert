#version 450 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;
layout(location = 2) in vec2 vTex;

out vec2 fTex;
out vec3 fNorm;
out vec3 fFragPos;
out vec4 fFragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;


void main()
{
    gl_Position = projection * view * model * vec4(vPos, 1.0);

    fFragPos = vec3(model * vec4(vPos, 1.0));
    fNorm = mat3(transpose(inverse(model))) * vNorm;
    fFragPosLightSpace = lightSpaceMatrix * vec4(fFragPos, 1.0);

    fTex = vTex;
}
