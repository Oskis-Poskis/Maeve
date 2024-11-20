#version 330 core
in vec2 TexCoords;
out vec4 fragColor;

uniform sampler2D text;
uniform vec3 color;

void main()
{    
    float d = texture(text, TexCoords).r;
    float aaf = fwidth(d);
    float alpha = smoothstep(0.5 - aaf, 0.5 + aaf, d);
    fragColor = vec4(color, alpha);
}  