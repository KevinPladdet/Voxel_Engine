#pragma once

#include <imgui.h>
#include <array>

#include "texture.h"
#include "block.h"
#include "inventory.h"

class UI_Manager
{
public:
	UI_Manager (int WINDOW_WIDTH, int WINDOW_HEIGHT, Inventory& inventory);
	~UI_Manager();

	void DrawUI();
	
	void DrawCrosshair();
	void DrawHotbar();

	void SetAtlasTile(int tileX, int tileY, ImVec2& uvMin, ImVec2& uvMax);
	ImVec2 GetItemCoordsForAtlas(Block::BlockType type);

	void DrawHotbarItems(float scale, ImVec2 hotbarStartPos);
	void DrawAtlasTile(ImVec2 drawStart, ImVec2 drawEnd, int gridX, int gridY);

private:
	int window_width = 0;
	int window_height = 0;

	// Hotbar
	Texture hotbarTexture;
	// Hotbar selection slot (left for 1, normal for 2-8, right for 9
	Texture hotbarSlotLeftTexture;
	Texture hotbarSlotTexture;
	Texture hotbarSlotRightTexture;

	// Items Atlas
	Texture itemsAtlasTexture;

	Inventory& m_inventory;
};