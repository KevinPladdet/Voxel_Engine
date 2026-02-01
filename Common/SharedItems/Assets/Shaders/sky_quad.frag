#version 310 es
precision mediump float;

in vec2 texCoord;

uniform sampler2D tex0;

out vec4 FragColor;

void main()
{
    vec4 tex = texture(tex0, texCoord);
    
    FragColor = vec4(tex.rgb, tex.a);
}