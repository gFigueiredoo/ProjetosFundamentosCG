#version 330 core
out vec4 FragColor;

uniform vec4 colorOverride;

void main()
{
    FragColor = colorOverride;
}