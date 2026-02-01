#include "EBO.h"
#include <assert.h>

// Constructor that generates a Elements Buffer Object and links it to indices
EBO::EBO(GLuint* indices, GLsizeiptr size)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
}

void EBO::Bind()
{
	assert(ID != -1 && "EBO NOT INITIALIZED");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

void EBO::Unbind()
{
	assert(ID != -1 && "EBO NOT INITIALIZED");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void EBO::Delete()
{
	assert(ID != -1 && "EBO NOT INITIALIZED");
	glDeleteBuffers(1, &ID);
}