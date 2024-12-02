#version 330 core
layout (location = 0) out vec4 fragColor;
in vec2 uvs;

const float PI = 3.14159265359;

uniform sampler2D GAlbedo;
uniform sampler2D GNormal;
uniform sampler2D GDepth;
uniform mat4 iProjMatrix;
uniform mat4 viewMatrix;
uniform vec3 cDir;

vec3 ViewPosFromDepth();

highp float NoiseCalc = 1.0 / 255;
highp float random(highp vec2 coords) { return fract(sin(dot(coords.xy, vec2(12.9898, 78.233))) * 43758.5453); }

void main()
{
    vec3 albedo   = texture(GAlbedo, uvs).rgb;
    vec3 normal   = texture(GNormal, uvs).rgb;
    vec3 viewPos  = ViewPosFromDepth();
    vec3 viewDir  = mat3(viewMatrix) * cDir;
    vec3 lightDir = mat3(viewMatrix) * vec3(1, 1, 0.5);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 final = albedo * (diff);

    // final = final / (final + 1.0);
    // final = pow(final, vec3(1.0 / 2.2));
    // final += mix(-NoiseCalc, NoiseCalc, random(uvs));

    fragColor = vec4(vec3(final), 1.0);
}

vec3 ViewPosFromDepth()
{
    float z = texture(GDepth, uvs).r * 2.0 - 1.0;
    vec4 clip_space_position = vec4(uvs * 2.0 - vec2(1.0), z, 1.0);
    vec4 position = iProjMatrix * clip_space_position;

    return position.xyz / position.w;
}