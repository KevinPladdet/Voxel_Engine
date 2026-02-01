#include "block.h"

void Block::SetAtlasTexture(FaceSide face, int tileX, int tileY)
{
    glm::vec2 offset(tileX * tileSize, (63 - tileY) * tileSize);

    if (face == ALL_FACES)
    {
        for (int i = 0; i < 6; ++i)
        {
            atlasOffset[i] = offset;
        }
    }
    else
    {
        atlasOffset[face] = offset;
    }
}

void Block::SetFaceColor(FaceSide face, float r, float g, float b)
{
    if (face == ALL_FACES)
    {
        for (int i = 0; i < 6; ++i)
        {
            faceColor[i] = glm::vec3(r, g, b);
        }
    }
    else
    {
        faceColor[face] = glm::vec3(r, g, b);
    }
}

void Block::SpawnBlock(BlockType type)
{
    // Set BlockType of this block as same type from parameter
    this->type = type;

    // Resets face colors back to default
    SetFaceColor(ALL_FACES, 1.0f, 1.0f, 1.0f);

	switch (type)
	{
    case AIR:
        break;

	case BEDROCK:
        SetAtlasTexture(ALL_FACES, 8, 9);
		break;

    case STONE:
        SetAtlasTexture(ALL_FACES, 26, 31);
        break;

    case DIRT:
        SetAtlasTexture(ALL_FACES, 28, 5);
        break;

    case GRASS:
        SetAtlasTexture(BACK_FACE, 6, 17);
        SetAtlasTexture(FRONT_FACE, 6, 17);
        SetAtlasTexture(LEFT_FACE, 6, 17);
        SetAtlasTexture(RIGHT_FACE, 6, 17);
        SetAtlasTexture(TOP_FACE, 9, 17);
        SetAtlasTexture(BOTTOM_FACE, 28, 5);

        SetFaceColor(TOP_FACE, 0.45f * rgbMultiplier, 0.7f * rgbMultiplier, 0.29f * rgbMultiplier);
        break;

    case SAND:
        SetAtlasTexture(ALL_FACES, 2, 29);
        break;

    case LAVA:
        SetAtlasTexture(ALL_FACES, 8, 2);
        break;

    case WATER:
        SetAtlasTexture(ALL_FACES, 10, 6);
        SetFaceColor(ALL_FACES, 0.25f * rgbMultiplier, 0.25f * rgbMultiplier, 0.7f * rgbMultiplier); // All Faces
        break;

    case OAK_LOG:
        SetAtlasTexture(BACK_FACE, 22, 22);
        SetAtlasTexture(FRONT_FACE, 22, 22);
        SetAtlasTexture(LEFT_FACE, 22, 22);
        SetAtlasTexture(RIGHT_FACE, 22, 22);
        SetAtlasTexture(TOP_FACE, 23, 22);
        SetAtlasTexture(BOTTOM_FACE, 23, 22);
        break;

    case OAK_PLANKS:
        SetAtlasTexture(ALL_FACES, 24, 22);
        break;

    case GLASS:
        SetAtlasTexture(ALL_FACES, 30, 16);
        break;

	default:
		printf("ERROR AT SpawnBlock() WITH SELECTION!\n");
		break;
	}
}