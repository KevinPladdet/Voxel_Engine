#include "VBO.h"
#include <assert.h>

// Constructor that generates a Vertex Buffer Object and links it to vertices
VBO::VBO(GLfloat* vertices, GLsizeiptr size)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

void VBO::Bind()
{
	assert(ID != -1 && "VBO NOT INITIALIZED");
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::Unbind()
{
	assert(ID != -1 && "VBO NOT INITIALIZED");
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::Delete()
{
	assert(ID != -1 && "VBO NOT INITIALIZED");
	glDeleteBuffers(1, &ID);
}