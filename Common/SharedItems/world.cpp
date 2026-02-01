#include "world.h"

void World::SetNoise()
{
	int randomSeed = rand() % 10000 + 1;
	printf("Random Seed: %d\n", randomSeed);

	noiseTerrain.SetNoiseType(FastNoiseLite::NoiseType_Perlin); // Forest biome
	noiseTerrain.SetSeed(randomSeed); // Default 1337
	noiseTerrain.SetFrequency(0.008f); // Default 0.01
	noiseTerrain.SetFractalType(FastNoiseLite::FractalType_FBm);
	noiseTerrain.SetFractalOctaves(5); // Amount of layers in noise

	noiseTerrainMix.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	noiseTerrainMix.SetSeed(randomSeed + 1);
	noiseTerrainMix.SetFrequency(0.002f);
	noiseTerrainMix.SetFractalType(FastNoiseLite::FractalType_FBm);
	noiseTerrainMix.SetFractalOctaves(2);
}

void World::SetCaveNoise()
{
	int randomSeed = rand() % 10000 + 1;
	printf("Random Caves Seed: %d\n", randomSeed);
	
	noiseCavesSimplex.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	noiseCavesSimplex.SetSeed(randomSeed);
	noiseCavesSimplex.SetFrequency(0.02f);
	noiseCavesSimplex.SetFractalType(FastNoiseLite::FractalType_FBm);
	noiseCavesSimplex.SetFractalOctaves(1);


	noiseCavesDetail.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	noiseCavesDetail.SetSeed(randomSeed + 1);
	noiseCavesDetail.SetFrequency(0.05f);
	noiseCavesDetail.SetFractalType(FastNoiseLite::FractalType_FBm);
	noiseCavesDetail.SetFractalOctaves(1);
}

void World::GenerateChunks()
{
	// Clear queue
	while (!queueChunks.empty())
	{
		queueChunks.pop();
	}

	std::vector<glm::ivec3> spiral = GenerateSpiral(renderDistance);

	// Generate chunks around the player with renderDistance value in spiral order
	for (auto& offset : spiral)
	{
		glm::ivec3 chunkPos = { playerChunkPos.x + offset.x, 0, playerChunkPos.z + offset.z };

		// Only add to queue if it isn't in chunkMap already
		if (chunkMap.find(chunkPos) == chunkMap.end())
		{
			queueChunks.push(chunkPos);
		}
	}
}

void World::DeleteChunks()
{
	std::vector<glm::ivec3> removeChunks;

	for (auto& chunk : chunkMap)
	{
		glm::ivec3 chunkPos = chunk.first;

		int x = chunkPos.x - playerChunkPos.x;
		int z = chunkPos.z - playerChunkPos.z;

		// If chunk is outside renderDistance, add chunkPos to removeChunks vector
		if (abs(x) > renderDistance || abs(z) > renderDistance)
		{
			removeChunks.push_back(chunkPos);
		}
	}

	// Delete every chunk added to removeChunks
	for (auto& chunkPos : removeChunks)
	{
		chunkMap[chunkPos].m_mesh.Delete();
		chunkMap[chunkPos].m_transparentMesh.Delete();
		chunkMap[chunkPos].m_glassMesh.Delete();
		chunkMap.erase(chunkPos);

		totalRenderedChunks--;
	}
}

// Spiral algorithm that returns ivec3 to generate chunks in spiral order
// I made GenerateSpiral with AI but understand how it works and could explain it as well
std::vector<glm::ivec3> World::GenerateSpiral(int radius)
{
	std::vector<glm::ivec3> result;
	result.reserve((radius * 2 + 1) * (radius * 2 + 1));

	int x = 0, z = 0; // Start at center (0,0)
	int dx = 1, dz = 0; // Move RIGHT first
	int segment_length = 1;
	int segment_passes = 0;

	for (int i = 0; i < result.capacity(); i++)
	{
		result.push_back(glm::ivec3(x, 0, z));

		x += dx;
		z += dz;
		
		segment_length--;

		// When segment is finished, rotate direction clockwise
		if (segment_length == 0)
		{
			int temp = dx;
			dx = -dz; // Rotate vector 90 degrees to the right
			dz = temp;

			segment_passes++;

			// Increase segment length every 2 turns
			segment_length = segment_passes / 2 + 1;

			// Stop at radius (renderDistance)
			if (abs(x) > radius || abs(z) > radius)
			{
				break;
			}
		}
	}

	return result;
}

Chunk* World::GetChunk(int x, int z)
{
	glm::ivec3 neighborPos(x, 0, z);

	// Try to find chunk
	auto it = chunkMap.find(neighborPos);

	// If not found, return nullptr
	if (it == chunkMap.end())
	{
		return nullptr;
	}

	// If found, return pointer to chunk
	return &it->second;
}

Block* World::GetBlock(int x, int y, int z)
{
	int chunkX = (int)(floor((float)x / Chunk::CHUNK_SIZE_X));
	int chunkZ = (int)(floor((float)z / Chunk::CHUNK_SIZE_Z));

	Chunk* c = GetChunk(chunkX, chunkZ);
	if (!c)
	{
		return nullptr;
	}

	int blockX = x - chunkX * Chunk::CHUNK_SIZE_X;
	int blockZ = z - chunkZ * Chunk::CHUNK_SIZE_Z;

	int worldY = y - Chunk::WORLD_Y_MIN;

	if (worldY < 0 || worldY >= Chunk::CHUNK_SIZE_Y)
	{
		return nullptr;
	}

	return &c->blocks[blockX][worldY][blockZ];
}

bool World::SetBlock(int x, int y, int z, Block::BlockType type)
{
	int chunkX = (int)(floor((float)x / Chunk::CHUNK_SIZE_X));
	int chunkZ = (int)(floor((float)z / Chunk::CHUNK_SIZE_Z));

	Chunk* c = GetChunk(chunkX, chunkZ);
	if (!c)
	{
		return false;
	}

	int blockX = x - chunkX * Chunk::CHUNK_SIZE_X;
	int blockZ = z - chunkZ * Chunk::CHUNK_SIZE_Z;

	int worldY = y - Chunk::WORLD_Y_MIN;

	// Make sure that block is within y limit
	if (worldY < 0 || worldY >= Chunk::CHUNK_SIZE_Y)
	{
		return false;
	}

	// Set block type to type given from parameter
	c->blocks[blockX][worldY][blockZ].type = type;

	OnBlockChanged(x, y, z);

	return true;
}

void World::OnBlockChanged(int x, int y, int z)
{
	int chunkX = (int)(floor((float)x / Chunk::CHUNK_SIZE_X));
	int chunkZ = (int)(floor((float)z / Chunk::CHUNK_SIZE_Z));

	Chunk* c = GetChunk(chunkX, chunkZ);

	// Rebuild mesh for chunk that the changed block is in
	c->BuildMesh(*this);

	int localX = x - chunkX * Chunk::CHUNK_SIZE_X;
	int localZ = z - chunkZ * Chunk::CHUNK_SIZE_Z;
	
	// Check if neighboring faces of changed block are in neighboring chunks
	if (localX == 0) 
		if (Chunk* leftNeighbor = GetChunk(chunkX - 1, chunkZ))
		{
			leftNeighbor->BuildMesh(*this);
		}
	if (localX == Chunk::CHUNK_SIZE_X - 1) 
		if (Chunk* rightNeighbor = GetChunk(chunkX + 1, chunkZ))
		{
			rightNeighbor->BuildMesh(*this);
		}
	if (localZ == 0) 
		if (Chunk* backNeighbor = GetChunk(chunkX, chunkZ - 1))
		{
			backNeighbor->BuildMesh(*this);
		}
	if (localZ == Chunk::CHUNK_SIZE_Z - 1) 
		if (Chunk* frontNeighbor = GetChunk(chunkX, chunkZ + 1))
		{
			frontNeighbor->BuildMesh(*this);
		}
}