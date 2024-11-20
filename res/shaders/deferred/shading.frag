#version 330 core

in vec2 uvs;
layout (location = 0) out vec4 fragColor;

const float PI = 3.14159265359;

uniform sampler2D GAlbedo;
uniform sampler2D GNormal;
uniform sampler2D GDepth;
uniform vec3      cPos;
uniform mat4 iProjMatrix;
uniform mat4 viewMatrix;

vec3 ViewPosFromDepth();

highp float NoiseCalc = 1.0 / 255;
highp float random(highp vec2 coords) { return fract(sin(dot(coords.xy, vec2(12.9898,78.233))) * 43758.5453); }

void main()
{
    vec3 albedo = texture(GAlbedo, uvs).rgb;
    vec3 N = texture(GNormal, uvs).rgb;
    vec3 P = ViewPosFromDepth();
    vec3 V = normalize(-P);

    vec3 VSLightPos = (viewMatrix * vec4(0, 8, 2, 1)).xyz;
    vec3 L = normalize(VSLightPos - P);

    float distance    = length(VSLightPos - P);
    float attenuation = 1.0 / (distance * distance);

    float specularStrength = 0.5;
    vec3 halfwayDir = normalize(L + V);
    float spec = pow(max(dot(N, halfwayDir), 0.0), 80);
    float specular = specularStrength * spec;

    float diff = max(dot(N, L), 0.0);

    vec3 final = albedo * (diff + specular);
    final *= attenuation;
    final = final / (final + 1.0);
    final = pow(final, vec3(1.0 / 2.2));
    final += mix(-NoiseCalc, NoiseCalc, random(uvs));

    fragColor = vec4(vec3(final), 1.0);
}

vec3 ViewPosFromDepth()
{
    float z = texture(GDepth, uvs).r * 2.0 - 1.0;
    vec4 clip_space_position = vec4(uvs * 2.0 - vec2(1.0), z, 1.0);
    vec4 position = iProjMatrix * clip_space_position;

    return position.xyz / position.w;
}