#include "inventory.h"

void Inventory::NavigateHotbar(bool hotbarSlotUp, bool usingNum, int slot)
{
	if (!usingNum) // Using mouse wheel
	{
		if (hotbarSlotUp)
		{
			selectedSlot += 1;
		}
		else
		{
			selectedSlot -= 1;
		}

		// Loop around hotbar
		if (selectedSlot > 9)
		{
			selectedSlot = 1;
		}
		if (selectedSlot < 1)
		{
			selectedSlot = 9;
		}
	}
	else // Using numbers 1-9
	{
		selectedSlot = slot;
	}
}

Block::BlockType Inventory::GetTypeFromSlot(int slot)
{
	// Returns BlockType of slot from parameter
	return hotbar[slot - 1];
}

void Inventory::SetSlotType(int slot, Block::BlockType type)
{
	// Changes hotbar slot from parameter to BlockType from parameter
	hotbar[slot - 1] = type;
}