#version 330 core
out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;

uniform vec3 viewPos;

void main()
{
    float specularStrength = 0.5;
    vec3 norm = normalize(normal);

    vec3 lightDir = normalize(vec3(-1.0, 1.0, 1.0));
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(normal, halfwayDir), 0.0), 80);
    float specular = specularStrength * spec;
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 final = vec3(diff);
    final = pow(final, vec3(1.0 / 2.2));

    FragColor = vec4(final, 1.0);
}