#pragma once

#include <array>
#include "block.h"

class Inventory
{
public:
	void NavigateHotbar(bool hotbarSlotUp, bool usingNum, int slot);
	Block::BlockType GetTypeFromSlot(int slot);
    void SetSlotType(int slot, Block::BlockType type);

	// Hotbar slot that is currently selected (1-9)
	int selectedSlot = 1;

private:
    // Contains what block each slot is holding
    std::array<Block::BlockType, 9> hotbar = 
    {
        Block::DIRT,       // 1
        Block::GRASS,      // 2
        Block::SAND,       // 3
        Block::OAK_LOG,    // 4
        Block::OAK_PLANKS, // 5
        Block::GLASS,      // 6
        Block::WATER,      // 7
        Block::STONE,      // 8
        Block::LAVA        // 9
    };
};