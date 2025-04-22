#version 330 core
layout (location = 3) out float gMask;

uniform int selected;

void main()
{
    gMask = 1.0;
}