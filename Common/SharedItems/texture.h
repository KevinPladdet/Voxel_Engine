#pragma once

#include "IncludeGraphics.h"
#include "stb/stb_image.h"

#include "shaderClass.h"

class Texture
{
public:
	GLuint ID;
	GLenum type;
	Texture(const char* image, GLenum textType, GLenum slot, GLenum format, GLenum pixeltype);

	void TexUnit(Shader& shader, const char* uniform, GLuint unit);
	void Bind();
	void Unbind();
	void Delete();
};