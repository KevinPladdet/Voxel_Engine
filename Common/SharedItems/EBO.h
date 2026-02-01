#pragma once

#include "IncludeGraphics.h"

class EBO
{
public:
	// ID reference of Elements Buffer Object
	GLuint ID;

	EBO() : ID(-1) {};
	// Constructor that generates a Elements Buffer Object and links it to indices
	EBO(GLuint* indices, GLsizeiptr size);

	void Bind();
	void Unbind();
	void Delete();
};