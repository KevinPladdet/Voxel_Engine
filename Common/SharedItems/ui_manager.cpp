#include "ui_manager.h"

UI_Manager::UI_Manager(int WINDOW_WIDTH, int WINDOW_HEIGHT, Inventory& inventory) :
	m_inventory(inventory),
	hotbarTexture("../Common/SharedItems/Assets/Textures/Hotbar.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE),
	hotbarSlotLeftTexture("../Common/SharedItems/Assets/Textures/Hotbar_Selection_Left.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE),
	hotbarSlotTexture("../Common/SharedItems/Assets/Textures/Hotbar_Selection.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE),
	hotbarSlotRightTexture("../Common/SharedItems/Assets/Textures/Hotbar_Selection_Right.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE),
	itemsAtlasTexture("../Common/SharedItems/Assets/Textures/Items_Atlas.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE)	
{
	UI_Manager::window_width = WINDOW_WIDTH;
	UI_Manager::window_height = WINDOW_HEIGHT;
}

UI_Manager::~UI_Manager()
{
	hotbarTexture.Delete();
	hotbarSlotLeftTexture.Delete();
	hotbarSlotTexture.Delete();
	hotbarSlotRightTexture.Delete();
	itemsAtlasTexture.Delete();
}

void UI_Manager::DrawUI()
{
	DrawCrosshair();
	DrawHotbar();
}

void UI_Manager::DrawCrosshair()
{
	ImDrawList* draw = ImGui::GetForegroundDrawList();
	// + 0.5f to make the crosshair EXACTLY in the middle of the screen #HalfPixelLivesMatter
	ImVec2 center = ImVec2(window_width / 2 + 0.5f, window_height / 2 + 0.5f);
	int size = 15;
	float thickness = 2.5f;
	int opacity = 200;
	draw->AddLine({ center.x - size, center.y }, { center.x + size, center.y }, IM_COL32(255, 255, 255, opacity), thickness);
	draw->AddLine({ center.x, center.y - size }, { center.x, center.y + size }, IM_COL32(255, 255, 255, opacity), thickness);
}

void UI_Manager::DrawHotbar()
{
	// Flipped UV (because for some reason ImGui draws it inverted)
	ImVec2 uv0(0.0f, 1.0f);
	ImVec2 uv1(1.0f, 0.0f);

	// Hotbar
	float scale = 2.5f;

	float hotbarPxWidth = 182.0f;
	float hotbarPxHeight = 24.0f;

	float hotbarWidth = hotbarPxWidth * scale;
	float hotbarHeight = hotbarPxHeight * scale;

	ImVec2 startPos(
		(window_width - hotbarWidth) / 2.0f,
		window_height - hotbarHeight);

	ImVec2 endPos(
		startPos.x + hotbarWidth,
		startPos.y + hotbarHeight);

	ImTextureID hotbarTexID = (ImTextureID)(intptr_t)hotbarTexture.ID;

	ImGui::GetForegroundDrawList()->AddImage(hotbarTexID, startPos, endPos, uv0, uv1);

	

	// Selected slot of hotbar
	const float slotPxSize = 20.0f;  // Slot size
	const float slotPxOffset = 1.0f; // Slot offset
	int slotIndex = m_inventory.selectedSlot - 1;

	float slotStartX = startPos.x + (slotPxOffset + slotIndex * slotPxSize) * scale;
	float slotStartY = startPos.y + slotPxOffset * scale;

	ImVec2 slotStart(slotStartX, slotStartY);
	
	ImVec2 slotEnd(
		slotStart.x + slotPxSize * scale,
		slotStart.y + slotPxSize * scale);

	// Margins for making the selected slot texture a little bit bigger
	float expandX = 5.0f;
	float expandTop = 5.0f;
	float expandBottom = 7.0f;

	slotStart.x -= expandX;
	slotEnd.x += expandX;
	slotStart.y -= expandTop;
	slotEnd.y += expandBottom;


	
	// Default slot texture for slots 2-8
	ImTextureID hotbarSelectedTexID = (ImTextureID)(intptr_t)hotbarSlotTexture.ID;

	// Slots 1 and 9 have added black border to their correspondent side
	if (m_inventory.selectedSlot == 1)
	{
		hotbarSelectedTexID = (ImTextureID)(intptr_t)hotbarSlotLeftTexture.ID;
	}
	else if (m_inventory.selectedSlot == 9)
	{
		hotbarSelectedTexID = (ImTextureID)(intptr_t)hotbarSlotRightTexture.ID;
	}

	ImGui::GetForegroundDrawList()->AddImage(hotbarSelectedTexID, slotStart, slotEnd, uv0, uv1);
	


	// Draw items inside of hotbar slots
	DrawHotbarItems(scale, startPos);
}

void UI_Manager::SetAtlasTile(int tileX, int tileY, ImVec2& uvMin, ImVec2& uvMax)
{
	const float totalTiles = 8.0f;
	const float tileSize = 1.0f / totalTiles;

	// Set uv positions to
	float uvTopLeft = tileX * tileSize;
	float uvBottomLeft = tileY * tileSize;
	float uvTopRight = uvTopLeft + tileSize;
	float uvBottomRight = uvBottomLeft + tileSize;
	
	uvMin = ImVec2(uvTopLeft, 1.0f - uvBottomLeft);
	uvMax = ImVec2(uvTopRight, 1.0f - uvBottomRight);
}

ImVec2 UI_Manager::GetItemCoordsForAtlas(Block::BlockType type)
{
	switch (type)
	{
		// First Row
		case Block::AIR:		return { 0, 0 };
		case Block::BEDROCK:	return { 1, 0 };
		case Block::DIRT:		return { 2, 0 };
		case Block::GRASS:		return { 3, 0 };
		case Block::SAND:		return { 4, 0 };
		case Block::STONE:		return { 5, 0 };
		case Block::WATER:		return { 6, 0 };
		case Block::LAVA:		return { 7, 0 };
		// Second Row
		case Block::OAK_LOG:    return { 0, 1 };
		case Block::OAK_PLANKS: return { 1, 1 };
		case Block::GLASS:		return { 2, 1 };
		default:				return { 0, 0 };
	}
}

void UI_Manager::DrawHotbarItems(float scale, ImVec2 hotbarStartPos)
{
	const float slotPxSize = 20.0f;  // Slot size
	const float slotPxOffset = 1.0f; // Slot offset
	const float itemPaddingPx = 2.0f; // Padding for item inside of slot

	for (int slot = 1;slot <= 9; slot++)
	{
		int i = slot - 1;

		float slotStartX = hotbarStartPos.x + (slotPxOffset + i * slotPxSize) * scale;
		float slotStartY = hotbarStartPos.y + slotPxOffset * scale;

		ImVec2 slotStart(slotStartX, slotStartY);
		
		ImVec2 slotEnd(
			slotStart.x + slotPxSize * scale,
			slotStart.y + slotPxSize * scale);
		
		ImVec2 itemStart(
			slotStart.x + itemPaddingPx * scale,
			slotStart.y + itemPaddingPx * scale);

		ImVec2 itemEnd(
			slotEnd.x - itemPaddingPx * scale,
			slotEnd.y - itemPaddingPx * scale);

		// Move all items inside of slots 1px downwards
		const float yOffsetPx = 1.0f;
		itemStart.y += yOffsetPx * scale;
		itemEnd.y += yOffsetPx * scale;

		// Set type to BlockType of slot
		Block::BlockType type = m_inventory.GetTypeFromSlot(slot);
		
		// Skip iteration if hotbar slot holds no item
		if (type == Block::AIR) continue;

		// Set texture coordinates for atlas
		ImVec2 texCoords = GetItemCoordsForAtlas(type);

		// Draw items inside of hotbar slots
		DrawAtlasTile(itemStart, itemEnd, (int)texCoords.x, (int)texCoords.y);
	}
}

void UI_Manager::DrawAtlasTile(ImVec2 drawStart, ImVec2 drawEnd, int gridX, int gridY)
{
	// Set uv's to item tile coords
	ImVec2 uvMin, uvMax;
	SetAtlasTile(gridX, gridY, uvMin, uvMax);

	// Set texture and draw item in hotbar slot
	ImTextureID itemsTexID = (ImTextureID)(intptr_t)itemsAtlasTexture.ID;
	ImGui::GetForegroundDrawList()->AddImage(itemsTexID, drawStart, drawEnd, uvMin, uvMax);
}