#version 330 core
layout (location = 4) out float out_ShadowFactor;
in vec2 uvs;

uniform sampler2D GNormal;
uniform sampler2D GDepth;

const int NUM_CASCADES = 3;
uniform mat4  lightSpaceMatrices[NUM_CASCADES];
uniform float cascadeSplits[NUM_CASCADES];
uniform sampler2DArray DirShadowMapRaw;
// uniform sampler2DArrayShadow DirShadowMap;

uniform mat4 iProjMatrix;
uniform mat4 viewMatrix;
uniform vec3 cDir;
uniform vec3 cPos;

uniform vec3 dirLightDir = vec3(1.0, -1.0, 0.75);

const float near = 0.1;
const float far  = 1000.0;
const float PI   = 3.1415926;

vec3 ViewPosFromDepth(float depth);

float CalcDirShadow(vec4 LSPos, vec3 normal, vec3 viewPos, int cascade);
float PCSS(vec4 LSPos, vec3 normal, int cascade);

highp float random(highp vec2 coords) { return fract(sin(dot(coords.xy, vec2(12.9898, 78.233))) * 43758.5453); }

int ChooseCascade(float viewDepth)
{
    for (int i = 0; i < NUM_CASCADES; i++) {
        if (viewDepth < cascadeSplits[i])
            return i;
    }
    return NUM_CASCADES - 1;
}

void main()
{
    vec3 normal = normalize(texture(GNormal, uvs).rgb);
    float depth = texture(GDepth, uvs).r;

    float ndc = texture(GDepth, uvs).r * 2.0 - 1.0;
    float linearDepth = (2.0 * near * far) / (far + near - ndc * (far - near));
    int   cascade = ChooseCascade(linearDepth);

    vec3 viewPos   = ViewPosFromDepth(depth);
    vec4 worldPos  = inverse(viewMatrix) * vec4(viewPos, 1.0);
    vec4 viewPosLS = lightSpaceMatrices[cascade] * worldPos;
    vec3 viewDir   = normalize(-viewPos);

    float dirLightShadow = PCSS(viewPosLS, normal, cascade);
    // float dirLightShadow = CalcDirShadow(viewPosLS, normal, viewPos, cascade);

    // out_ShadowFactor = texture(DirShadowMapRaw, vec3(uvs, cascade)).r;
    out_ShadowFactor = dirLightShadow;
}

const vec2 poissonDisk[32] = vec2[](
    vec2(-0.613, -0.538),
    vec2( 0.296, -0.757),
    vec2( 0.669, -0.174),
    vec2( 0.186,  0.560),
    vec2(-0.720,  0.128),
    vec2(-0.207, -0.207),
    vec2( 0.486,  0.383),
    vec2(-0.097,  0.843),
    vec2( 0.864,  0.146),
    vec2( 0.205, -0.408),
    vec2(-0.355,  0.671),
    vec2(-0.591, -0.029),
    vec2(-0.191,  0.247),
    vec2( 0.079,  0.020),
    vec2( 0.421, -0.544),
    vec2(-0.438, -0.741),
    vec2( 0.732,  0.642),
    vec2(-0.893,  0.315),
    vec2(-0.203, -0.923),
    vec2( 0.310,  0.921),
    vec2( 0.095, -0.984),
    vec2(-0.987, -0.054),
    vec2(-0.657,  0.607),
    vec2( 0.531, -0.847),
    vec2( 0.947, -0.260),
    vec2(-0.389,  0.910),
    vec2( 0.743, -0.666),
    vec2(-0.031,  0.987),
    vec2( 0.914,  0.392),
    vec2(-0.798, -0.495),
    vec2(-0.120, -0.582),
    vec2( 0.267,  0.305)
);

vec2 VogelDiskSample(int sampleIndex, int samplesCount, float phi)
{
  float GoldenAngle = 2.4;

  float r = sqrt(sampleIndex + 0.5) / sqrt(samplesCount);
  float theta = sampleIndex * GoldenAngle + phi;

  float sine = sin(theta);
  float cosine = cos(theta);
  
  return vec2(r * cosine, r * sine);
}

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float getWorldTexelSize(mat4 lightViewProj, int shadowMapRes) {
    // For ortho: projection matrix scale X = 2 / (right - left)
    float projScaleX = lightViewProj[0][0]; // assumes no weird skew
    float orthoWidth = 2.0 / projScaleX;
    return orthoWidth / float(shadowMapRes);
}

float findBlocker(vec2 uv, float zReceiver, float searchRadius, vec3 normal, int cascade)
{
    int NUM_SAMPLES = 16;

    int texSize = textureSize(DirShadowMapRaw, 0).x;
    float angle = rand(vec2(floor(uv * texSize * 8))) * 6.2831853;
    mat2  rot   = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    float randAngle = rand(uv * textureSize(DirShadowMapRaw, 0).x);

    float avgBlockerDepth = 0.0;
    int   blockers = 0;

    vec3  lightDir = mat3(viewMatrix) * normalize(dirLightDir);
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        vec2 offset = rot * VogelDiskSample(i, NUM_SAMPLES, randAngle * 6.2831);
        float depth = texture(DirShadowMapRaw, vec3(uv + offset * searchRadius, float(cascade))).r;

        if (depth < zReceiver) {
            avgBlockerDepth += depth;
            blockers++;
        }
    }

    return (blockers > 0) ? avgBlockerDepth / float(blockers) : -1.0;
}

float PCSS(vec4 LSPos, vec3 normal, int cascade)
{
    vec3 projCoords = LSPos.xyz / LSPos.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) return 1.0;

    float zReceiver = projCoords.z;
    int texSize     = textureSize(DirShadowMapRaw, 0).x;
    float texelSize = 1.0 / texSize;
    
    float lightSize     = 0.5;
    float searchRadius  = 12.0;
    float jitterRadius  = 256.0; // 24.0
    float penumbraPower = 0.85;
    float shadowPower   = 1.25;
    int   NUM_SAMPLES   = 16;

    // Step 1: Blocker search (using raw depth texture)
    float avgBlockerDepth = findBlocker(projCoords.xy, zReceiver, lightSize * texelSize * searchRadius, normal, cascade);
    if (avgBlockerDepth < 0.0) return 1.0;

    // Step 2: Estimate penumbra size
    float rawPenumbra = (zReceiver - avgBlockerDepth) * lightSize / avgBlockerDepth;
    float softenedPenumbra = pow(rawPenumbra, penumbraPower);
    // Mix between raw and softened penumbra — stronger raw near contact
    float blend = smoothstep(0.001, 0.025, rawPenumbra);
    float penumbra = mix(rawPenumbra, softenedPenumbra, blend);

    // Step 3: PCF filter
    float shadow = 0.0;
    float totalWeight = 0.0;

    vec3 lightDir = mat3(viewMatrix) * normalize(dirLightDir);
    float bias = max(0.0025 * (1.0 - dot(normal, lightDir)), 0.00005);

    if (cascade == NUM_CASCADES)
        bias *= 1 / (far * 0.5);
    else
        bias *= 1 / (cascadeSplits[cascade] * 0.5);

    // float angle = rand(gl_FragCoord.xy) * 6.2831853;
    float angle = rand(vec2(floor(projCoords.xy * textureSize(GNormal, 0) * 16))) * 6.2831853;
    mat2  rot   = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));

    for (int i = 0; i < NUM_SAMPLES; i++) {
        vec2 offsetVec = rot * VogelDiskSample(i, NUM_SAMPLES, rand(projCoords.xy) * 6.2831);
        vec2 offset = offsetVec * penumbra * texelSize * jitterRadius;

        float dist = length(offsetVec);
        float sigma = 0.8;
        float weight = exp(-(dist * dist) / (2.0 * sigma * sigma));
        weight = 1.0;

        float sampledDepth = texture(DirShadowMapRaw, vec3(projCoords.xy + offset, float(cascade))).r;
        float visibility = sampledDepth < zReceiver - bias ? 0.0 : 1.0;
        shadow += (1.0 - visibility) * weight;

        totalWeight += weight;
    }

    // return penumbra;

    shadow /= totalWeight;
    shadow = pow(shadow, shadowPower);
    return 1.0 - shadow;
}

float CalcDirShadow(vec4 LSPos, vec3 normal, vec3 viewPos, int cascade)
{
    int NUM_SAMPLES = 12;

    vec3 projCoords = LSPos.xyz / LSPos.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) return 1.0;

    float currentDepth = projCoords.z;

    vec3 lightDir = mat3(viewMatrix) * normalize(dirLightDir);
    float bias = max(0.0025 * (1.0 - dot(normal, lightDir)), 0.00005);

    if (cascade == NUM_CASCADES)
        bias *= 1 / (far * 0.5);
    else
        bias *= 1 / (cascadeSplits[cascade] * 0.5);

    ivec2 texSize   = textureSize(DirShadowMapRaw, 0).xy;
    vec2 texelSize = 1.0 / texSize;

    float shadow = 0.0;
    float angle = rand(vec2(floor(projCoords.xy * textureSize(GNormal, 0) * 16))) * 6.2831853;
    mat2  rot   = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));

    for (int i = 0; i < NUM_SAMPLES; i++) {
        float texelWorldSize = getWorldTexelSize(lightSpaceMatrices[cascade], texSize.x);
        float uvRadius = 0.025 / texelWorldSize; // KERNEL_RADIUS in world units
        vec2  offset = rot * VogelDiskSample(i, NUM_SAMPLES, rand(projCoords.xy) * 6.2831853) * uvRadius;

        float sampledDepth = texture(DirShadowMapRaw, vec3(projCoords.xy + offset * texelSize * 2.0, float(cascade))).r;
        float visibility = sampledDepth < projCoords.z - bias ? 0.0 : 1.0;
        shadow += visibility;
    }
    shadow /= NUM_SAMPLES;

    return shadow;
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