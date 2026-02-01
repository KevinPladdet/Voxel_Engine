#pragma once

#include "IncludeGraphics.h"

class VBO
{
public:
	// Reference ID of the Vertex Buffer Object
	GLuint ID;
	
	VBO() : ID(-1) {};
	// Constructor that generates a Vertex Buffer Object and links it to vertices
	VBO(GLfloat* vertices, GLsizeiptr size);

	void Bind();
	void Unbind();
	void Delete();
};