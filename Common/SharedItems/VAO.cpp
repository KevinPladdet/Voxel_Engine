#include "VAO.h"

// Constructor that generates a VAO ID
VAO::VAO()
{
	glGenVertexArrays(1, &ID);
}

// Links a VBO to VAO using layout
void VAO::LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizei stride, void* offset)
{
	VBO.Bind();
	glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);
	VBO.Unbind();
}

void VAO::Bind()
{
	glBindVertexArray(ID);
}

void VAO::Unbind()
{
	glBindVertexArray(0);
}

void VAO::Delete()
{
	glDeleteVertexArrays(1, &ID);
}