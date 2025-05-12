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

#define MAX_POINT_LIGHTS 64
uniform PointLight PointLights[MAX_POINT_LIGHTS];
uniform int NumPointLights;

uniform sampler2D GAlbedo;
uniform sampler2D GNormal;
uniform sampler2D GDepth;
uniform sampler2D GDirShadowFactor;

uniform mat4 iProjMatrix;
uniform mat4 viewMatrix;
uniform vec3 cDir;
uniform vec3 cPos;

const vec3 BGcol = vec3(0.025, 0.025, 0.025);
const float near = 0.1;
const float far  = 1000.0;
const float PI   = 3.1415926;

vec3 ViewPosFromDepth(float depth);

vec3  fresnelSchlick(float cosTheta, vec3 F0);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

vec3 CalcDirLight(vec3 albedo, vec3 normal, float metallic, float roughness, float ao, vec3 viewPos, vec3 viewDir, float shadow);

vec3 CalcPointLight(PointLight light, vec3 albedo, vec3 normal, float metallic, float roughness, float ao, vec3 viewPos, vec3 viewDir);
vec3 CalcPointLightPhong(PointLight light, vec3 albedo, vec3 normal, vec3 viewpos, vec3 viewdir);

highp float NoiseCalc = 1.0 / 255;
highp float random(highp vec2 coords) { return fract(sin(dot(coords.xy, vec2(12.9898, 78.233))) * 43758.5453); }

float BlurShadowFactor(sampler2D shadowTex, vec2 uv, vec2 texelSize)
{
    float kernel[25] = float[](
        1.0,  4.0,  6.0,  4.0, 1.0,
        4.0, 16.0, 24.0, 16.0, 4.0,
        6.0, 24.0, 36.0, 24.0, 6.0,
        4.0, 16.0, 24.0, 16.0, 4.0,
        1.0,  4.0,  6.0,  4.0, 1.0
    );

    float sum = 0.0;
    float weightSum = 0.0;
    int i = 0;

    for (int y = -2; y <= 2; ++y) {
        for (int x = -2; x <= 2; ++x) {
            vec2 offset = vec2(x, y) * texelSize;
            float sample = texture(shadowTex, uv + offset).r;
            float weight = kernel[i++];
            sum += sample * weight;
            weightSum += weight;
        }
    }

    return sum / weightSum;
}

void main()
{
    // vec3  albedo    = texture(GAlbedo, uvs).rgb;
    vec3  albedo    = vec3(0.80, 0.65, 0.60);
    vec3  normal    = normalize(texture(GNormal, uvs).rgb);
    float metallic  = 0.0;
    float roughness = 0.25;
    float ao        = 0.0;
    float depth = texture(GDepth, uvs).r;

    vec3 viewPos   = ViewPosFromDepth(depth);
    vec4 worldPos  = inverse(viewMatrix) * vec4(viewPos, 1.0);
    vec3 viewDir = normalize(-viewPos);

    // vec2 texelSize = 1.0 / textureSize(GDirShadowFactor, 0);
    // float dirShadow = BlurShadowFactor(GDirShadowFactor, uvs, texelSize);
    float dirShadow = texture(GDirShadowFactor, uvs).r;
    float shadowStrength = 0.95;

    vec3 ambient = BGcol * albedo;
    vec3 pointLighting = vec3(0.0);
    for (int i = 0; i < NumPointLights; i++) {
        pointLighting += CalcPointLight(PointLights[i], albedo, normal, metallic, roughness, ao, viewPos, viewDir);
    }

    vec3 dirLighting = CalcDirLight(albedo, normal, metallic, roughness, ao, viewPos, viewDir, 1.0);
    // dirLighting *= mix(1.0, dirShadow, shadowStrength);

    vec3 final = ambient + pointLighting + dirLighting;

    final  = final / (final + 1.0); // simple reinhardt tonemap
    final  = pow(final, vec3(1.0 / 2.2));
    final += mix(-NoiseCalc, NoiseCalc, random(uvs));

    fragColor = vec4(final, 1.0);
    if (depth > 0.9999) fragColor = vec4(BGcol, 1.0);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 CalcPointLight(PointLight light, vec3 albedo, vec3 normal, float metallic, float roughness, float ao, vec3 viewPos, vec3 viewDir)
{
    vec3 N = normal;
    vec3 V = normalize(viewDir);
    vec3 dir = mat3(viewMatrix) * light.position - (viewPos + mat3(viewMatrix) * cPos);
    vec3 L = normalize(dir);
    vec3 H = normalize(V + L);
    float len = length(dir);
    float attenuation = 1.0 / (len * len);
    vec3 radiance = light.color * light.intensity * attenuation;

    // Fresnel reflectance at normal incidence
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3  nom   = NDF * G * F;
    float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3  specular = nom / denom;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 CalcDirLight(vec3 albedo, vec3 normal, float metallic, float roughness, float ao, vec3 viewPos, vec3 viewDir, float shadow)
{
    vec3 N = normal;
    vec3 V = normalize(viewDir);
    vec3 dir = mat3(viewMatrix) * normalize(vec3(1.0, -1.0, 0.75));
    vec3 L = normalize(dir);
    vec3 H = normalize(V + L);
    float len = length(dir);
    float attenuation = 1.0 / (len * len);
    vec3 radiance = vec3(7.5);

    // Fresnel reflectance at normal incidence
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3  nom   = NDF * G * F;
    float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3  specular = nom / denom;
    specular = specular * shadow;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

// ------------------------------------------------------------------

vec3 ViewPosFromDepth(float depth)
{
    vec2 ndc = uvs * 2.0 - 1.0;
    vec4 clip = vec4(ndc, depth * 2.0 - 1.0, 1.0);
    vec4 view = iProjMatrix * clip;
    return view.xyz / view.w;
}

// ------------------------------------------------------------------

vec3 CalcPointLightPhong(PointLight light, vec3 albedo, vec3 normal, vec3 viewPos, vec3 viewDir)
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