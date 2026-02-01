#pragma once

#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include <vector>

class Mesh
{
public:
	void Link(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices);
	void Draw();
	void Delete();

	static int totalVertexCount;

	static bool polygonLines;

private:
	VAO m_VAO;
	VBO m_VBO;
	EBO m_EBO;

	GLsizei indexCount = 0;

	int vertexCount = 0;
};