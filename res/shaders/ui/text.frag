#version 330 core
in vec2 TexCoords;
out vec4 fragColor;

uniform sampler2D text;
uniform vec3 color;

void main()
{    
    float d = texture(text, TexCoords).r;
    float aaf = fwidth(d);
    float test = 0.5;
    float alpha = smoothstep(test - aaf, test + aaf, d);
    fragColor = vec4(color, alpha);
}  