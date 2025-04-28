#version 330 core

in vec2 uvs;
out vec4 fragColor;

uniform vec3 color;
uniform float blend;

uniform float outerRadiusPX;
uniform float innerRadiusPX;

void main()
{
    vec2 centeredUV = uvs - vec2(0.5);
    float dist = length(centeredUV);

    float outerRadius = 0.5;
    float innerRadius = (innerRadiusPX / outerRadiusPX) / 2.0;
    float thickness = 0.01;

    // Smooth outer radius
    float alpha = smoothstep(outerRadius, outerRadius - thickness, dist);
    // Smooth inner radius
    alpha -= smoothstep(innerRadius + thickness, innerRadius, dist);

    if (alpha <= 0.0) discard;

    fragColor = vec4(color, alpha);
}
