#version 310 es
precision highp float;

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertTex;

uniform mat4 camMatrix;
uniform mat4 model;

out vec2 texCoord;

void main()
{
    gl_Position = camMatrix * model * vec4(vertPos, 1.0f);
    texCoord = vertTex;
}