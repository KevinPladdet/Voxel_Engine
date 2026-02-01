#version 310 es
precision mediump float;

out vec4 FragColor;

uniform vec4 outlineColor;

void main()
{
    FragColor = outlineColor;
}