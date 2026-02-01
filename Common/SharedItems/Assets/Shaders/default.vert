#version 310 es
precision highp float;

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertTex;
layout (location = 2) in vec2 atlasOffset;
layout (location = 3) in vec3 faceColor;
layout (location = 4) in vec3 normals;

// Outputs to default.frag
out vec2 texCoord;
out vec3 outFaceColor;
out vec3 outNormals;

uniform mat4 camMatrix;

// Needed to select specific block from Blocks_Atlas.png
uniform vec2 faceOffset[6];
uniform vec2 faceScale[6];

void main()
{
	// Outputs the positions of all vertices
	gl_Position = camMatrix * vec4(vertPos, 1.0f);
	
	// Set texture coords with offset to get correct texture from atlas
	const float tileSize = 1.0f / 64.0f;
	texCoord = vertTex * tileSize + atlasOffset;

	// Pass vFaceColor to default.frag
	outFaceColor = faceColor;

	// Pass normals to default.frag
	outNormals = normals;
};