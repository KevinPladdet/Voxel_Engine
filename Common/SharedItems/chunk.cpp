#include "chunk.h"
#include "world.h"

static const glm::vec3 vertexes[6][4] =
{
	{   // Back      (face 0)
		{0.0f, 0.0f, 0.0f},
		{1.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 0.0f},
		{0.0f, 1.0f, 0.0f}
	},

	{   // Front     (face 1)
		{ 0.0f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 1.0f, 1.0f }
	},
	
	{   // Left      (face 2)
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f }
	},
	
	{   // Right     (face 3)
		{ 1.0f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f }
	},
	   
	{   // Top       (face 4)
		{ 0.0f, 1.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 1.0f, 1.0f }
	},
	
	{   // Bottom    (face 5)
		{ 0.0f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f }
	}
};

static const glm::vec2 uvPositions[4] =
{
	{0.0f, 0.0f},
	{1.0f, 0.0f},
	{1.0f, 1.0f},
	{0.0f, 1.0f}
};

static const uint8_t faceIndices[6][6] =
{
	// Back   (face 0)
	{0, 1, 2, 
	 2, 3, 0},

	// Front  (face 1)
	{2, 1, 0,
	 0, 3, 2},

	// Left   (face 2)
	{2, 1, 0,
	 0, 3, 2},

	// Right  (face 3)
	{2, 1, 0,
	 0, 3, 2},

	// Top    (face 4)
	{0, 1, 3,
	 1, 2, 3},

	// Bottom (face 5)
	{1, 3, 0,
	 2, 3, 1}
};

static const glm::vec3 normals[6] =
{
	{  0.0f,  0.0f, -1.0f }, // Back      (face 0)
	{  0.0f,  0.0f,  1.0f }, // Front     (face 1)
	{ -1.0f,  0.0f,  0.0f }, // Left      (face 2)
	{  1.0f,  0.0f,  0.0f }, // Right     (face 3)
	{  0.0f,  1.0f,  0.0f }, // Top       (face 4)
	{  0.0f, -1.0f,  0.0f }  // Bottom    (face 5)
};

static bool IsTransparent(Block::BlockType type)
{
	return type == Block::AIR || type == Block::WATER || type == Block::GLASS;
}

void Chunk::GenerateBlocks(FastNoiseLite& noiseTerrain, FastNoiseLite& noiseTerrainMix, FastNoiseLite& noiseCavesSimplex, FastNoiseLite& noiseCavesDetail, int chunkPosX, int chunkPosZ, bool renderCaves)
{
	chunkPos = { chunkPosX / CHUNK_SIZE_X, 0, chunkPosZ / CHUNK_SIZE_Z };

	for (int x = 0; x < CHUNK_SIZE_X; ++x)
	{
		for (int z = 0; z < CHUNK_SIZE_Z; ++z)
		{
			// Get world position of each block in chunk
			float worldX = (float)(chunkPosX + x);
			float worldZ = (float)(chunkPosZ + z);

			// Terrain Noise
			float n = noiseTerrain.GetNoise(worldX, worldZ);
			float n2 = noiseTerrainMix.GetNoise(worldX * 0.5f, worldZ * 0.5f);

			// Increase mountains
			n2 = n2 + 1.2f;

			float terrainHeight = 0.0f; // Starting height of terrain noise
			float balance = 75.0f; // Balance between flat and mountains
			float amplitude = balance + n2;
			int noiseHeight = (int)(n * amplitude + terrainHeight);

			for (int y = 0; y < CHUNK_SIZE_Y; ++y)
			{
				int worldY = y + WORLD_Y_MIN;

				Block& b = blocks[x][y][z];

				// Used to generate bedrock
				int bedrockTop = WORLD_Y_MIN + 4;
				if (worldY <= bedrockTop) // Generate Bedrock
				{
					int bedrockLayers = 5;
					float scalar = 16 / (float)bedrockLayers;
					float chance = (worldY - WORLD_Y_MIN) * scalar;
					int random = rand() & 15;

					if (random >= chance)
					{
						b.type = Block::BEDROCK;
					}
					else
					{
						b.type = Block::STONE;
					}
				}
				// Above terrain = air
				else if (worldY > noiseHeight)
				{
					b.type = Block::AIR;
				}
				// Surface = grass
				else if (worldY == noiseHeight)
				{
					b.type = Block::GRASS;
				}
				// Below surface = dirt
				else if (worldY > noiseHeight - 3)
				{
					b.type = Block::DIRT;
				}
				// Everything else = stone
				else
				{
					b.type = Block::STONE;
				}
				
				// Caves
				if (renderCaves)
				{
					if (b.type == Block::STONE || b.type == Block::DIRT || b.type == Block::GRASS)
					{
						float simplexCaveNoise = noiseCavesSimplex.GetNoise(worldX, (float)worldY, worldZ);
						float perlinCaveNoise = noiseCavesDetail.GetNoise(worldX, (float)worldY, worldZ);

						float threshold = 0.08f;

						// Make caves bigger the deeper they are
						float depthBelow = (float)(-20 - worldY);
						if (worldY < -20)
						{
							threshold += (depthBelow / 44.0f) * 0.12f;
						}

						if (std::abs(simplexCaveNoise) < threshold && std::abs(perlinCaveNoise) < threshold)
						{
							b.type = Block::AIR;
						}
					}
				}
			}
			
			// Oceans
			int waterLevel = -20 - WORLD_Y_MIN;
			for (int y = waterLevel; y >= 0; --y)
			{
				Block& b = blocks[x][y][z];

				// Replace grass with sand
				if (b.type == Block::GRASS)
				{
					b.type = Block::SAND;

					// Replace the 2 layers under sand if it's dirt
					for (int depth = 1; depth <= 2; depth++)
					{
						if (y - depth >= 0 && blocks[x][y - depth][z].type == Block::DIRT)
						{
							blocks[x][y - depth][z].type = Block::SAND;
						}
					}
				}

				// Create first/top layer of water
				if (blocks[x][waterLevel][z].type == Block::AIR)
				{
					blocks[x][waterLevel][z].type = Block::WATER;
				}
			}

			// Fill all air under water with water
			for (int y = waterLevel - 1; y >= 0; --y)
			{
				Block& b = blocks[x][y][z];
				Block& above = blocks[x][y + 1][z];

				if (b.type == Block::AIR && above.type == Block::WATER)
				{
					b.type = Block::WATER;
				}
			}
		}
	}
}

void Chunk::BuildMesh(World& world)
{
	// Used for normal blocks that aren't transparent
	vertexData.clear();
	indexData.clear();
	GLuint indexOffset = 0;

	// Used for water
	transparentVertexData.clear();
	transparentIndexData.clear();
	GLuint transparentIndexOffset = 0;

	// Used for glass
	glassVertexData.clear();
	glassIndexData.clear();
	GLuint glassIndexOffset = 0;

	for (int x = 0; x < CHUNK_SIZE_X; ++x)
	{
		for (int y = 0; y < CHUNK_SIZE_Y; ++y)
		{
			for (int z = 0; z < CHUNK_SIZE_Z; ++z)
			{
				Block& b = blocks[x][y][z];

				// Don't continue with AIR blocks
				if (b.type == Block::AIR)
				{
					continue;
				}

				b.SpawnBlock(b.type);

				glm::vec3 worldPos(
					chunkPos.x * CHUNK_SIZE_X + x,
					y + WORLD_Y_MIN,
					chunkPos.z * CHUNK_SIZE_Z + z
				);

				for (int face = 0; face < 6; ++face)
				{
					if (!FaceIsVisible(x, y, z, face, world))
					{
						continue;
					}
					
					// Set vectors correctly depending on block type
					std::vector<GLfloat>* targetVertexData;
					std::vector<GLuint>* targetIndexData;
					GLuint* targetOffset;

					if (b.type == Block::WATER)
					{
						targetVertexData = &transparentVertexData;
						targetIndexData = &transparentIndexData;
						targetOffset = &transparentIndexOffset;
					}
					else if (b.type == Block::GLASS)
					{
						targetVertexData = &glassVertexData;
						targetIndexData = &glassIndexData;
						targetOffset = &glassIndexOffset;
					}
					else
					{
						targetVertexData = &vertexData;
						targetIndexData = &indexData;
						targetOffset = &indexOffset;
					}

					for (int i = 0; i < 4; ++i)
					{
						glm::vec3 v = vertexes[face][i] + worldPos;

						// Position
						targetVertexData->push_back(v.x);
						targetVertexData->push_back(v.y);
						targetVertexData->push_back(v.z);
						// Texture Coordinates / UVs
						targetVertexData->push_back(uvPositions[i].x);
						targetVertexData->push_back(uvPositions[i].y);
						// Atlas Offset (Used to select specific texture from atlas png)
						targetVertexData->push_back(b.atlasOffset[face].x);
						targetVertexData->push_back(b.atlasOffset[face].y);
						// Color
						glm::vec3 c = b.faceColor[face];
						targetVertexData->push_back(c.r);
						targetVertexData->push_back(c.g);
						targetVertexData->push_back(c.b);
						// Normals
						targetVertexData->push_back(normals[face].x);
						targetVertexData->push_back(normals[face].y);
						targetVertexData->push_back(normals[face].z);
					}

					for (int i = 0; i < 6; i++)
					{
						targetIndexData->push_back(*targetOffset + faceIndices[face][i]);
					}

					*targetOffset += 4; // += 4 because each face uses 4 vertices
				}
			}
		}
	}

	m_mesh.Link(vertexData, indexData);
	m_transparentMesh.Link(transparentVertexData, transparentIndexData);
	m_glassMesh.Link(glassVertexData, glassIndexData);
}

bool Chunk::FaceIsVisible(int x, int y, int z, int face, World& world)
{
	// Neighbor direction for each face
	static const int dir[6][3] =
	{
		{ 0,  0, -1}, // Back
		{ 0,  0,  1}, // Front
		{-1,  0,  0}, // Left
		{ 1,  0,  0}, // Right
		{ 0,  1,  0}, // Top
		{ 0, -1,  0}  // Bottom
	};

	int nx = x + dir[face][0];
	int ny = y + dir[face][1];
	int nz = z + dir[face][2];

	// Don't make bottom face of bedrock visible
	if (y == 0 && face == 5)
	{
		return false;
	}

	// Get current block type
	Block::BlockType currentType = blocks[x][y][z].type;

	// Check if current block is transparent

	// If neighbor face is inside of chunk and is air or water, make face visible
	if (nx >= 0 && nx < CHUNK_SIZE_X &&
		ny >= 0 && ny < CHUNK_SIZE_Y &&
		nz >= 0 && nz < CHUNK_SIZE_Z)
	{
		Block::BlockType neighborType = blocks[nx][ny][nz].type;
		
		// Water doesn't show faces against water
		if (currentType == Block::WATER && neighborType == Block::WATER)
		{
			return false;
		}

		// Water only shows faces against air
		if (currentType == Block::WATER)
		{
			return neighborType == Block::AIR || neighborType == Block::GLASS;
		}

		// Glass shows faces against everything except air
		if (currentType == Block::GLASS)
		{
			return neighborType != Block::GLASS;
		}

		// Regular blocks show faces against transparent blocks
		return IsTransparent(neighborType);
	}


	// Everything inside the chunk is now returned true or false 
	// the rest down here is for neighboring chunks
	bool left = nx < 0;
	bool right = nx >= CHUNK_SIZE_X;
	bool back = nz < 0;
	bool front = nz >= CHUNK_SIZE_Z;
	
	int cx = chunkPos.x;
	int cz = chunkPos.z;

	// Set left/right/back/front to respective neighboring chunks
	if (left) { cx -= 1; }
	else if (right) { cx += 1; }
	if (back) { cz -= 1; }
	else if (front) { cz += 1; }

	Chunk* neighbor = world.GetChunk(cx, cz);

	// If neighbor doesn't exist, return false
	if (!neighbor)
	{
		return false;
	}

	if (left) { nx = CHUNK_SIZE_X - 1; }
	else if (right) { nx = 0; }
	if (back) { nz = CHUNK_SIZE_Z - 1; }
	else if (front) { nz = 0; }

	// If top face is at the height limit, make face visible
	if (ny >= CHUNK_SIZE_Y && face == 4)
	{
		return true;
	}
	
	// Get BlockType of neighbor
	Block::BlockType neighborType = neighbor->blocks[nx][ny][nz].type;

	// Water doesn't show faces against water
	if (currentType == Block::WATER && neighborType == Block::WATER)
	{
		return false;
	}

	// Water only shows faces against air
	if (currentType == Block::WATER)
	{
		return neighborType == Block::AIR || neighborType == Block::GLASS;
	}

	// Glass shows faces against everything except air
	if (currentType == Block::GLASS)
	{
		return neighborType != Block::GLASS;
	}

	// Regular blocks show faces against transparent blocks
	return IsTransparent(neighborType);
}