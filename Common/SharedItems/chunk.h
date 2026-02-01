#pragma once

#include "block.h"
#include "mesh.h"
#include <vector>
#include <Assets/Lib/FastNoiseLite.h>
#include <glm/vec3.hpp>

class World;
class Chunk
{
public:
	void GenerateBlocks(FastNoiseLite& noiseTerrain, FastNoiseLite& noiseTerrainMix, FastNoiseLite& noiseCavesSimplex, FastNoiseLite& noiseCavesDetail, int chunkPosX, int chunkPosZ, bool renderCaves);
	void BuildMesh(World& world);
	bool FaceIsVisible(int x, int y, int z, int face, World& world);

	static constexpr int CHUNK_SIZE_X = 16;
	static constexpr int CHUNK_SIZE_Y = 128;
	static constexpr int WORLD_Y_MIN = -64;
	static constexpr int CHUNK_SIZE_Z = 16;

	Block blocks[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];
	
	// Mesh for normal blocks
	Mesh m_mesh;

	// Mesh for transparent blocks like water (drawn after m_mesh)
	std::vector<GLfloat> transparentVertexData;
	std::vector<GLuint> transparentIndexData;
	Mesh m_transparentMesh;

	// Mesh for glass (drawn after m_transparentMesh)
	std::vector<GLfloat> glassVertexData;
	std::vector<GLuint> glassIndexData;
	Mesh m_glassMesh;

private:
	std::vector<GLfloat> vertexData;
	std::vector<GLuint> indexData;

	glm::ivec3 chunkPos;
};