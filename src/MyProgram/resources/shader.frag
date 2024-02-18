#version 450 core

in vec2 fTex;
in vec3 fNorm;
in vec3 fFragPos;
in vec4 fFragPosLightSpace;

out vec4 fragColor;

uniform vec3 camPos;
uniform vec3 lightPos;

uniform vec3 lightColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;

vec2 poissonDisk[16] = vec2[](
    vec2(-0.94201624, -0.39906216),
    vec2(0.94558609, -0.76890725),
    vec2(-0.094184101, -0.92938870),
    vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432),
    vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845),
    vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554),
    vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),
    vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507),
    vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641367),
    vec2(0.14383161, -0.14100790));

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i)
{
    vec4 seed4 = vec4(seed, i);
    float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
    return fract(sin(dot_product) * 43758.5453);
}


float ShadowCalc(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if (projCoords.z > 1.0)
    {
        return 0.0;
    }

    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // check whether current frag pos is in shadow
    vec3 lightDir = normalize(lightPos - fFragPos);
    float bias = max(0.05 * (1.0 - dot(fNorm, lightDir)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    int pcfCount = 2;

    for (int x = -pcfCount; x <= pcfCount; ++x)
    {
        for (int y = -pcfCount; y <= pcfCount; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= (pcfCount * 2.0 + 1) * (pcfCount * 2.0 + 1);

    return shadow;
}

void main()
{
    vec3 objectColor = texture(texture_diffuse1, fTex).rgb;

    vec3 ambient = 0.1 * lightColor;

    vec3 lightDir = normalize(lightPos - fFragPos);

    vec3 diffuse = max(dot(fNorm, lightDir), 0.0) * lightColor;

    vec3 viewDir = normalize(0.0 - fFragPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(fNorm, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;

    float shadow = ShadowCalc(fFragPosLightSpace);

    fragColor = vec4(
        (ambient + (1 - shadow) * (diffuse + specular)) * objectColor,
        1.0);
}