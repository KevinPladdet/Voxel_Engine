#version 310 es
precision highp float;

layout(location = 0) in vec3 outlinePos;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(outlinePos, 1.0);
}