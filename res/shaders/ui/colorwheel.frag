#version 330 core

in vec2 uvs;
out vec4 fragColor;

uniform float outerRadiusPX;
uniform float innerRadiusPX;

vec3 HSVtoRGB(vec3 hsv)
{
    float h = hsv.x;
    float s = hsv.y;
    float v = hsv.z;

    float c = v * s;
    float x = c * (1.0 - abs(mod(h / 60.0, 2.0) - 1.0));
    float m = v - c;

    vec3 rgb;

    if (h < 60.0)       rgb = vec3(c, x, 0);
    else if (h < 120.0) rgb = vec3(x, c, 0);
    else if (h < 180.0) rgb = vec3(0, c, x);
    else if (h < 240.0) rgb = vec3(0, x, c);
    else if (h < 300.0) rgb = vec3(x, 0, c);
    else                rgb = vec3(c, 0, x);

    return rgb + vec3(m);
}

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

    float angle = degrees(atan(centeredUV.y, centeredUV.x));
    if (angle < 0.0) angle += 360.0;

    vec3 color = HSVtoRGB(vec3(angle, 1.0, 1.0));

    fragColor = vec4(color, alpha);
}