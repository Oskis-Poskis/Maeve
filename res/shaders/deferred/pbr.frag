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

const float roughness = 0.1;
const float metallic  = 0.0;

vec3 ViewPosFromDepth();

float DistributionGGX(vec3 N, vec3 H, float a);
float GeometrySchlickGGX(float NdotV, float k);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k);
vec3  FresnelSchlick(float cosTheta, vec3 F0);

highp float NoiseCalc = 1.0 / 255;
highp float random(highp vec2 coords) { return fract(sin(dot(coords.xy, vec2(12.9898,78.233))) * 43758.5453); }

void main()
{
    vec3 albedo = texture(GAlbedo, uvs).rgb;
    vec3 N = mat3(viewMatrix) * texture(GNormal, uvs).rgb;
    vec3 P = ViewPosFromDepth();
    vec3 V = normalize(-P);
    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    //
    vec3 VSLightPos = (viewMatrix * vec4(0, 8, 2, 1)).xyz;
    vec3 L = normalize(VSLightPos - P);
    //vec3 L = normalize(mat3(viewMatrix) * vec3(1));
    vec3 H = normalize(V + L);
    float distance    = length(VSLightPos - P);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = vec3(5.0) * attenuation;

    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3  numerator   = vec3(1) * NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3  specular    = numerator / denominator;

    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;
    //

    vec3 ambient = vec3(0.0) * albedo;
    vec3 color   = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    color += mix(-NoiseCalc, NoiseCalc, random(uvs));

    fragColor = vec4(color, 1.0);
}

vec3 ViewPosFromDepth()
{
    float z = texture(GDepth, uvs).r * 2.0 - 1.0;
    vec4 clip_space_position = vec4(uvs * 2.0 - vec2(1.0), z, 1.0);
    vec4 position = iProjMatrix * clip_space_position;

    return position.xyz / position.w;
}

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1  = GeometrySchlickGGX(NdotV, k);
    float ggx2  = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}