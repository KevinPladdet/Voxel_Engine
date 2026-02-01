#include "mesh.h"
#include <cstdio>
int Mesh::totalVertexCount = 0;
bool Mesh::polygonLines = false;

void Mesh::Link(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices)
{
	// Remove vertexes if mesh binded for this chunk before already
	totalVertexCount -= vertexCount;

	// Delete VBO and EBO if they already exist to prevent memory usage piling up when generating new chunks
	if (m_VBO.ID != (GLuint)-1)
	{
		m_VBO.Delete();
		m_VBO.ID = -1;
	}
	if (m_EBO.ID != (GLuint)-1)
	{
		m_EBO.Delete();
		m_EBO.ID = -1;
	}

	m_VAO.Bind();

	// Generates Vertex Buffer Object and links it to vertices
	m_VBO = VBO(vertices.data(), vertices.size() * sizeof(GLfloat));
	// Generates Element Buffer Object and links it to indices
	m_EBO = EBO(indices.data(), indices.size() * sizeof(GLuint));

	// Links VBO to VAO
	m_VAO.LinkAttrib(m_VBO, 0, 3, GL_FLOAT, 13 * sizeof(float), (void*)0); // Position
	m_VAO.LinkAttrib(m_VBO, 1, 2, GL_FLOAT, 13 * sizeof(float), (void*)(3 * sizeof(float))); // UV Coords
	m_VAO.LinkAttrib(m_VBO, 2, 2, GL_FLOAT, 13 * sizeof(float), (void*)(5 * sizeof(float))); // atlasOffset
	m_VAO.LinkAttrib(m_VBO, 3, 3, GL_FLOAT, 13 * sizeof(float), (void*)(7 * sizeof(float))); // faceColor
	m_VAO.LinkAttrib(m_VBO, 4, 3, GL_FLOAT, 13 * sizeof(float), (void*)(10 * sizeof(float))); // normals


	// Unbind all to prevent accidentally modifying them
	m_VAO.Unbind();

	indexCount = static_cast<GLsizei>(indices.size());

	vertexCount = static_cast<int>(vertices.size());
	totalVertexCount += vertexCount;
}

void Mesh::Draw()
{
	m_VAO.Bind();
	if (!polygonLines)
	{
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	}
	else
	{
		glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_INT, 0);
	}
	m_VAO.Unbind();
}

void Mesh::Delete()
{
	totalVertexCount -= vertexCount;
	vertexCount = 0;

	m_VAO.Delete();
	m_VBO.Delete();
	m_EBO.Delete();
}