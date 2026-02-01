#pragma once

#include "chunk.h"
#include <Assets/Lib/FastNoiseLite.h>
#include <queue>
#include <glm/vec3.hpp>
#include <unordered_map>

// Needed for hash functions for vector
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

class World
{
public:
	void SetNoise();
	void SetCaveNoise();
	void GenerateChunks();
	void DeleteChunks();
	std::vector<glm::ivec3> GenerateSpiral(int radius);
	Chunk* GetChunk(int x, int z);
	Block* GetBlock(int x, int y, int z);
	bool SetBlock(int x, int y, int z, Block::BlockType type);
	void OnBlockChanged(int x, int y, int z);
	
	glm::ivec3 playerChunkPos;
	glm::ivec3 previousPlayerChunkPos = glm::ivec3(INT_MAX);

	FastNoiseLite noiseTerrain;
	FastNoiseLite noiseTerrainMix;

	FastNoiseLite noiseCavesSimplex;
	FastNoiseLite noiseCavesDetail;

	// Temporarely holds all to be generated chunkPos values and are removed after they are generated
	std::queue<glm::ivec3> queueChunks;

	// Holds chunkPos as first value and Chunk instantiations as second value for every chunk active
	std::unordered_map<glm::ivec3, Chunk> chunkMap;

	int totalRenderedChunks = 0; // Used to display totalRenderedChunks with ImGUI
	
	#ifdef WINDOWS_BUILD
	int renderDistance = 10;
	bool renderCaves = true;
	#endif
	#ifdef Raspberry_BUILD
	int renderDistance = 1;
	bool renderCaves = false;
	#endif
};