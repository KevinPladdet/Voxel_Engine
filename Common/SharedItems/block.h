#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderClass.h"

class Block
{
public:

    enum FaceSide
    {
        BACK_FACE,
        FRONT_FACE,
        LEFT_FACE,
        RIGHT_FACE,
        TOP_FACE,
        BOTTOM_FACE,
        ALL_FACES
    };

    enum BlockType
    {
        AIR,
        BEDROCK,
        STONE,
        DIRT,
        GRASS,
        SAND,
        LAVA,
        WATER,
        OAK_LOG,
        OAK_PLANKS,
        GLASS
    };

    Block()
    {
        // Set atlasOffset and faceColor to default values
        for (int i = 0; i < 6; ++i)
        {
            atlasOffset[i] = glm::vec2(0.0f, 0.0f);
            faceColor[i] = glm::vec3(1.0f, 1.0f, 1.0f);
        }
    }
	void SetAtlasTexture(FaceSide face, int tileX, int tileY);
    void SetFaceColor(FaceSide face, float r, float g, float b);
	void SpawnBlock(BlockType type);
    
    glm::vec2 atlasOffset[6];
    glm::vec3 faceColor[6];

    FaceSide face;
    BlockType type;

private:
    static constexpr float tileSize = 1.0f / 64.0f;
    const float rgbMultiplier = 1.75;
};