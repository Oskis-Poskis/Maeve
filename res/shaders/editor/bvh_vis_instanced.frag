#version 330 core
out vec4 FragColor;

uniform vec3 color;

flat in int instanceID;

void main()
{
    float r = float(instanceID) / (499761.0 / 10);  // Use a smaller number for quick debug
    float g = float(instanceID) / (499761.0);  // Different scale for variety
    float b = float(instanceID) / (499761.0 * 2);
    FragColor = vec4(r, g, b, 1.0);   
}