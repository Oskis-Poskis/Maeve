#version 330 core
layout (location = 0) out vec4 fragColor;
in vec2 uvs;

struct PointLight
{
    vec3  position;
    float radius;
    vec3  color;
    float intensity;
};

#define MAX_POINT_LIGHTS 32
uniform PointLight PointLights[MAX_POINT_LIGHTS];
uniform int NumPointLights;

uniform sampler2D GAlbedo;
uniform sampler2D GNormal;
uniform sampler2D GDepth;

uniform mat4 iProjMatrix;
uniform mat4 viewMatrix;
uniform vec3 cDir;
uniform vec3 cPos;

const vec3 BG = vec3(0.1, 0.1, 0.1);
const float near = 0.1;
const float far  = 1000.0;

vec3 ViewPosFromDepth();
vec3 CalcPointLight(PointLight light, vec3 albedo, vec3 normal, vec3 viewpos, vec3 viewdir);

highp float NoiseCalc = 1.0 / 255;
highp float random(highp vec2 coords) { return fract(sin(dot(coords.xy, vec2(12.9898, 78.233))) * 43758.5453); }

void main()
{
    vec3 albedo  = texture(GAlbedo, uvs).rgb;
    vec3 normal  = texture(GNormal, uvs).rgb;
    vec3 viewPos = ViewPosFromDepth();
    vec3 viewDir = normalize(-viewPos);

    vec3 final;
    for (int i = 0; i < NumPointLights; i++)
    {
        final += CalcPointLight(PointLights[i], albedo, normal, viewPos, viewDir);
    }
    
    // final = final / (final + 1.0); // simple reinhardt tonemap
    final  = pow(final, vec3(1.0 / 2.2));
    final += mix(-NoiseCalc, NoiseCalc, random(uvs));

    fragColor = vec4(final, 1.0);
    if (normal.x == 1.0 && normal.y == 1.0 && normal.z == 1.0) fragColor = vec4(BG, 1.0);
    else if (normal.x == 0.0 && normal.y == 0.0 && normal.z == 0.0) fragColor = vec4(BG, 1.0);
}

vec3 ViewPosFromDepth()
{
    float depth = texture(GDepth, uvs).r;

    vec2 ndc = uvs * 2.0 - 1.0;
    vec4 clip = vec4(ndc, depth * 2.0 - 1.0, 1.0);
    vec4 view = iProjMatrix * clip;
    return view.xyz / view.w;
}

vec3 CalcPointLight(PointLight light, vec3 albedo, vec3 normal, vec3 viewPos, vec3 viewDir)
{
    vec3 dist = mat3(viewMatrix) * light.position - (viewPos + mat3(viewMatrix) * cPos);
    vec3 lightDir = normalize(dist); // normalize(mat3(viewMatrix) * vec3(1.0, 0.75, 0.5));

    float lambertian = max(dot(normal, lightDir), 0.0);
    float specular = 0.0;
    if (lambertian > 0.0)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        specular = pow(max(dot(normal, halfwayDir), 0.0), 80) * 0.75;
    }

    float len = length(dist);
    float attenuation = 1.0 / (len * len);

    return albedo * (specular * light.intensity + lambertian * light.color * light.intensity) * attenuation;
}