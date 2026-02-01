#version 310 es
precision mediump float;

// Inputs the texture coordinates from default.vert
in vec2 texCoord;
in vec3 outFaceColor;
in vec3 outNormals;

uniform sampler2D tex0;

// Outputs colors in RGBA
out vec4 FragColor;

// Lighting
uniform vec3 SunDir;

void main()
{
	vec4 texSample = texture(tex0, texCoord);

    // Lighting
    float ambient = 0.5f; // Higher = brighter shadows
    float diffuse = max(dot(normalize(outNormals), normalize(SunDir)), 0.0f);
    float lighting = ambient + diffuse * 0.5f;

    // Apply lighting and color to texture
    FragColor = vec4(texSample.rgb * outFaceColor * lighting, texSample.a);
}