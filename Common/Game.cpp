#include "Game.h"
#include "Input.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
#include "IGraphics.h"
#include "IInput.h"

#include "texture.h"
#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

#ifdef _DEBUG
	#include "vld.h"
#endif

#pragma region outlineVertexes
// This is used for the highlighted black outlines on the block that the player is looking at
static GLfloat outlineVertexes[] = 
{
	// Bottom square
	0,0,0,  1,0,0,
	1,0,0,  1,0,1,
	1,0,1,  0,0,1,
	0,0,1,  0,0,0,

	// Top square
	0,1,0,  1,1,0,
	1,1,0,  1,1,1,
	1,1,1,  0,1,1,
	0,1,1,  0,1,0,

	// Vertical edges
	0,0,0,  0,1,0,
	1,0,0,  1,1,0,
	1,0,1,  1,1,1,
	0,0,1,  0,1,1,
};

// Vertex count of outlineVerts
static const int outlineVertexCount = (int)(sizeof(outlineVertexes) / (3 * sizeof(GLfloat)));
#pragma endregion

Game::Game(const Input* const input, IGraphics* graphics) :
	input(input),
	graphics(graphics),
	m_ui_manager(WINDOW_WIDTH, WINDOW_HEIGHT, m_inventory)
{
	
}

Game::~Game()
{
	// Todo:
	// - Swimming in water
	//
	// - AudioManager
	//  - Open inventory SFX
	
	// Todo for when all from above is completed:
	// - Trees
	// - Healthbar
	// - Inventory
	// - Show block/item bottom right in hand
	// - Toggle in ImGui to show chunk borders
	// - Good caves generation with smooth cave openings at the surface
	
	// If I had more time:
	// - Frustum culling
}



void Game::Start()
{
	InitializeOpenGLES();
	printf("This GPU supplied by  : %s\n", glGetString(GL_VENDOR));
	printf("This GPU supports GL  : %s\n", glGetString(GL_VERSION));
	printf("This GPU Renders with : %s\n", glGetString(GL_RENDERER));
	printf("This GPU Shaders are  : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("---------------------------------------------\n");

	// Timing
	auto startTime = std::chrono::system_clock::now();
	auto lastTime = startTime;

	float averageFPS{ 0 };

	#ifdef WINDOWS_BUILD
	// Lock mouse cursor
	graphics->LockMouse();
	#endif

	// Initialize ImGUI
	graphics->InitImGUI();

	// Seed rng
	std::srand((int)(std::time(nullptr)));

	// Generates Shader object using shaders default.vert and default.frag
	Shader shaderProgram("../Common/SharedItems/Assets/Shaders/default.vert", "../Common/SharedItems/Assets/Shaders/default.frag");

	// Generates Shader object using outline.vert and outline.frag
	Shader outlineShader("../Common/SharedItems/Assets/Shaders/outline.vert", "../Common/SharedItems/Assets/Shaders/outline.frag");
	// Create cube for outlines
	m_outlineVAO.Bind();
	m_outlineVBO = VBO(outlineVertexes, sizeof(outlineVertexes));
	m_outlineVAO.LinkAttrib(m_outlineVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
	m_outlineVAO.Unbind();

	// Blocks Texture (Atlas)
	Texture blocksTexture("../Common/SharedItems/Assets/Textures/Blocks_Atlas.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
	blocksTexture.TexUnit(shaderProgram, "tex0", 0);

	m_world.SetNoise();
	m_world.SetCaveNoise();

	// Randomized background music
	m_audiomanager.PlayRandomMusic(1.0f);

	m_sky_renderer.Init();

	// Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST);

	// Set camera position and window sizes
	m_player = Player(WINDOW_WIDTH, WINDOW_HEIGHT, glm::vec3(0.0f, 30.0f, 0.0f), m_world, m_audiomanager);
	
	while(!quitting)
	{
		graphics->BeginFrameImGUI();

		ProcessInput();
		auto time = std::chrono::system_clock::now();
		std::chrono::duration<float> delta = time - lastTime;

		gameDeltaTime = delta.count();

		std::chrono::duration<float> elapsed = time - startTime;
		if(elapsed.count() > 0.25f && frameCount > 10)
		{
			averageFPS = static_cast<float>(frameCount) / elapsed.count();

			startTime = time;
			frameCount = 0;
		}

		DayNightCycle();

		// Check if song has stopped playing
		m_audiomanager.ChangeMusic();

		ClearScreen();
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

		// !! GAME UPDATE LOOP STARTS HERE !!

		shaderProgram.Activate();

		GLint sunDir = glGetUniformLocation(shaderProgram.ID, "SunDir");
		glUniform3f(sunDir, sunDirection.x, sunDirection.y, sunDirection.z);
		
		// Bind texture and Vertex Array Buffer
		blocksTexture.Bind();

		#pragma region Player Movement
		// Handles looking around with mouse inputs
		m_player.MouseInput(gameDeltaTime, GetInput());
		m_player.Update(shaderProgram, "camMatrix", gameDeltaTime);
		
		// Set playerChunkPosition (ONLY USED TO SHOW IN IMGUI DEBUG WINDOW)
		m_world.playerChunkPos = 
		{ 
			static_cast<int>(std::floor(m_player.GetPosition().x / Chunk::CHUNK_SIZE_X)),
			0,
			static_cast<int>(std::floor(m_player.GetPosition().z / Chunk::CHUNK_SIZE_Z))
		};
		#pragma endregion

		// Render Sun & Moon
		m_sky_renderer.Render(m_player.GetPosition(), sunDirection, dayFactor, sunAngle, m_player.view, m_player.projection);

		// Reactivate block shader after rendering the Sun & Moon
		shaderProgram.Activate();
		blocksTexture.Bind();

		#pragma region Chunk Generation
		// Only update chunks when player has entered a new chunk
		if (m_world.playerChunkPos != m_world.previousPlayerChunkPos)
		{
			m_world.previousPlayerChunkPos = m_world.playerChunkPos;
			m_world.GenerateChunks();
			m_world.DeleteChunks();
		}

		// Checking Chunk generation queue
		if (!m_world.queueChunks.empty())
		{
			glm::ivec3 chunkPos = m_world.queueChunks.front();
			m_world.queueChunks.pop();
			
			int chunkPosX = chunkPos.x * Chunk::CHUNK_SIZE_X;
			int chunkPosZ = chunkPos.z * Chunk::CHUNK_SIZE_Z;

			Chunk& createdChunk = m_world.chunkMap.emplace(std::piecewise_construct, std::make_tuple(chunkPos), std::make_tuple()).first->second;
			
			createdChunk.GenerateBlocks(m_world.noiseTerrain, m_world.noiseTerrainMix, m_world.noiseCavesSimplex, m_world.noiseCavesDetail, chunkPosX, chunkPosZ, m_world.renderCaves);
			createdChunk.BuildMesh(m_world);
			m_world.totalRenderedChunks++;

			// Direction of neighboring chunks
			static const glm::ivec3 dir[4] = 
			{
				{ 1,0,0 }, {-1,0,0 },
				{ 0,0,1 }, { 0,0,-1 }
			};

			for (int i = 0; i < 4; i++)
			{
				glm::ivec3 neighborPos = chunkPos + dir[i];
				Chunk* neighborChunk = m_world.GetChunk(neighborPos.x, neighborPos.z);
				if (neighborChunk)
				{
					neighborChunk->BuildMesh(m_world);
				}	
			}
		}
		#pragma endregion

		#pragma region Draw blocks & block outlines
		// Draw all blocks
		for (std::unordered_map<glm::ivec3, Chunk>::iterator it = m_world.chunkMap.begin();
			it != m_world.chunkMap.end(); ++it)
		{
			it->second.m_mesh.Draw();
		}

		// Draw transparent blocks (like water) after all other blocks (without writing to depth buffer)
		glDepthMask(GL_FALSE);
		for (std::unordered_map<glm::ivec3, Chunk>::iterator it = m_world.chunkMap.begin();
			it != m_world.chunkMap.end(); ++it)
		{
			it->second.m_transparentMesh.Draw();
		}

		// Draw glass after all normal blocks and transparent blocks have drawn (without writing to depth buffer)
		for (std::unordered_map<glm::ivec3, Chunk>::iterator it = m_world.chunkMap.begin();
			it != m_world.chunkMap.end(); ++it)
		{
			it->second.m_glassMesh.Draw();
		}
		glDepthMask(GL_TRUE);

		// Draw outlines if looking at block
		if (m_player.isLookingAtBlock)
		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(m_player.lookingAtBlockPos));
			model = glm::scale(model, glm::vec3(1.001f)); // Scale by a bit to prevent "z fighting"

			// Model view projection matrix
			glm::mat4 mvp = m_player.projection * m_player.view * model;
			
			outlineShader.Activate();

			// Change mvp in shader to local mvp from above
			GLint mvpShader = glGetUniformLocation(outlineShader.ID, "mvp");
			glUniformMatrix4fv(mvpShader, 1, GL_FALSE, glm::value_ptr(mvp));

			// Set color of outlines to black
			GLint shaderColor = glGetUniformLocation(outlineShader.ID, "outlineColor");
			glUniform4f(shaderColor, 0.0f, 0.0f, 0.0f, 1.0f);
			
			glEnable(GL_DEPTH_TEST);
			glLineWidth(2.5f); // Increase thickness of lines by 2.5
			
			m_outlineVAO.Bind();
			glDrawArrays(GL_LINES, 0, outlineVertexCount);
			m_outlineVAO.Unbind();
			
			glLineWidth(1.0f); // Set thickness of lines back to default just in case its used somewhere else
		}
		#pragma endregion

		// UI
		m_ui_manager.DrawUI();

		// Debug Menu (F3 to toggle)
		if (enable_debugging)
		{
			ImGUI(averageFPS);
			m_audiomanager.ChangeVolumes();
		}

		graphics->EndFrameImGUI();
		
		graphics->SwapBuffer();
		lastTime = time;
		++frameCount;
	}

	// Deletes VAO, VBO & EBO
	for (std::unordered_map<glm::ivec3, Chunk>::iterator it = m_world.chunkMap.begin();
		it != m_world.chunkMap.end(); ++it)
	{
		it->second.m_mesh.Delete();
		it->second.m_transparentMesh.Delete();
		it->second.m_glassMesh.Delete();
	}

	blocksTexture.Delete();
	shaderProgram.Delete();
	graphics->ShutDownImGUI();
}

void Game::ImGUI(float fps)
{
	// Create ImGui window to top left (position 10, 10)
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
	ImGui::Begin("Debug", nullptr, 
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse);

	ImGui::SeparatorText("General");
	#pragma region FPS
	static float fpsGraph[200] = {};
	static int fpsIndex = 0;

	fpsGraph[fpsIndex] = fps;
	fpsIndex = (fpsIndex + 1) % 200;

	float maxFPS = 240.0f;
	float padding = 130.0f;

	ImGui::Text("FPS: %.0f", fps);
	ImGui::PlotLines("##FPS Graph", fpsGraph, 200, fpsIndex, "FPS Graph", 0.0f, maxFPS + padding, ImVec2(0, 50));
	#pragma endregion
	
	ImGui::SeparatorText("Player");
	ImGui::DragFloat3("Position", &m_player.playerPos.x);
	ImGui::BeginDisabled();
	ImGui::DragFloat("Speed", &m_player.currentSpeed, 0.1f, 0.0f, 0.0f, "%.1f");
	ImGui::DragInt3("Chunk Position", &m_world.playerChunkPos.x);
	//ImGui::DragInt3("Looking At", &m_player.lookingAtBlockPos.x);
	//ImGui::Text("Hotbar Slot: %d", m_inventory.selectedSlot);
	ImGui::EndDisabled();

	ImGui::SeparatorText("World");
	ImGui::DragFloat("View Distance", &m_player.farPlane, 0.1f, 0.0f, 0.0f, "%.0f blocks");
	#pragma region RenderDistance
	ImGui::Text("Render Distance: %d", m_world.renderDistance);
	ImGui::SameLine();

	ImGui::BeginDisabled(m_world.renderDistance <= 0);
	if (ImGui::Button("-1"))
	{
		m_world.renderDistance -= 1;
	}
	ImGui::EndDisabled();

	ImGui::SameLine();

	if (ImGui::Button("+1"))
	{
		m_world.renderDistance += 1;
	}
	#pragma endregion
	ImGui::Text("Total Chunks: %d", m_world.totalRenderedChunks);
	ImGui::Text("Total Vertexes: %d", m_chunk.m_mesh.totalVertexCount);
	ImGui::Checkbox("Show Polygon Lines", &m_chunk.m_mesh.polygonLines);
	
	ImGui::SeparatorText("Generation");
	ImGui::Checkbox("Render Caves", &m_world.renderCaves);

	ImGui::SeparatorText("Creative");
	#pragma region Gamemode Switching
	bool creativeChanged = ImGui::Checkbox("Enable Creative", &m_player.inCreative);

	if (creativeChanged)
	{
		if (m_player.inCreative)
		{
			m_player.SetCreative();
		}
		else
		{
			m_player.SetSurvival();
		}
	}
	#pragma endregion
	ImGui::BeginDisabled();
	ImGui::Checkbox("Flying", &m_player.isFlying);
	ImGui::EndDisabled();

	ImGui::SeparatorText("Sounds");
	ImGui::SliderFloat("SFX Volume", &m_audiomanager.sfxVolume, 0.0f, 100.0f, "%.0f%%");
	ImGui::SliderFloat("Music Volume", &m_audiomanager.musicVolume, 0.0f, 100.0f, "%.0f%%");

	ImGui::SeparatorText("Other");
	ImGui::DragFloat("Day Duration", &day_duration, 0.1f, 0.0f, 0.0f, "%.0f seconds");

	ImGui::End();
}

const Input& Game::GetInput() const
{
	return *input;
}

void Game::Quit()
{
	quitting = true;
}

void Game::ProcessInput()
{
	const Input& input = GetInput();
	const IMouse& mouse = GetInput().GetMouse();
	
	// Timer that never gets added to or subtracted from
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	// If player hits head, add interval to timer
	if (m_player.hitHead)
	{
		headhitterTimer = now + HEADHITTER_INTERVAL;
	}

	// Debugging
	if (KeyPressedOnce(Key::F3, keyUpF3))
	{
		enable_debugging = !enable_debugging;
	}

	// Escape key to close program
	if (input.GetKeyboard().GetKey(Key::ESCAPE))
	{
		Quit();
	}

	// Movement Input
	bool isMoving = false;
	bool isHoldingW = false;
		
	if (input.GetKeyboard().GetKey(Key::W))
	{
		isMoving = true;
		isHoldingW = true;
		m_player.Inputs(m_player.FORWARD, gameDeltaTime, GetInput());
	}
	if (input.GetKeyboard().GetKey(Key::S))
	{
		isMoving = true;
		m_player.Inputs(m_player.BACKWARD, gameDeltaTime, GetInput());
	}
	if (input.GetKeyboard().GetKey(Key::A))
	{
		isMoving = true;
		m_player.Inputs(m_player.LEFT, gameDeltaTime, GetInput());
	}
	if (input.GetKeyboard().GetKey(Key::D))
	{
		isMoving = true;
		m_player.Inputs(m_player.RIGHT, gameDeltaTime, GetInput());
	}
	
	// If holding any movement key (WASD), set holdingMoveInput to true
	m_player.holdingMoveInput = isMoving;

	// Jump Input
	if (input.GetKeyboard().GetKey(Key::SPACE))
	{
		if (m_player.isFlying)
		{
			m_player.Inputs(m_player.JUMP, gameDeltaTime, GetInput());
		}
		else if (now >= headhitterTimer)
		{
			m_player.Inputs(m_player.JUMP, gameDeltaTime, GetInput());
		}
	}
	else
	{
		headhitterTimer = now;
	}

	// Crouching Input
	if (input.GetKeyboard().GetKey(Key::CTRL_LEFT))
	{
		m_player.isCrouching = true;
	}
	else
	{
		m_player.isCrouching = false;
	}

	// Sprinting Input
	if (input.GetKeyboard().GetKey(Key::SHIFT_LEFT) && isHoldingW)
	{
		m_player.isSprinting = true;
	}
	else
	{
		m_player.isSprinting = false;
	}

	#pragma region Mouse Input
	// Left Click
	if (input.GetMouse().GetButtonDown(MouseButtons::LEFT))
	{
		if (m_player.isLookingAtBlock)
		{
			if (now >= destroyBlockTimer)
			{
				m_player.DestroyBlock(Block::AIR);
				destroyBlockTimer = now + DESTROY_INTERVAL;
			}
		}
	}
	else
	{
		destroyBlockTimer = now;
	}

	// Right Click
	if (input.GetMouse().GetButtonDown(MouseButtons::RIGHT))
	{
		if (m_player.isLookingAtBlock)
		{
			if (now >= placeBlockTimer)
			{
				m_player.PlaceBlock(m_inventory.GetTypeFromSlot(m_inventory.selectedSlot));
				placeBlockTimer = now + PLACE_INTERVAL;
			}
		}
	}
	else
	{
		placeBlockTimer = now;
	}
	#pragma endregion
		
	// Inventory & Lock Mouse
	if (KeyPressedOnce(Key::E, keyUpE))
	{
		m_player.mouseLocked = !m_player.mouseLocked;

		LockMouseWindows();

		// add inventory here so it opens/draws the inventory png
		// prevent movement when in inventory
		// prevent camera turning when in inventory
	}

	#pragma region Hotbar Navigation
	// Hotbar Navigation
	float mousewheel = mouse.CheckScrollDelta();

	// For reading clarity
	bool usingNum = true;
	bool hotbarSlotDown = false;
	bool hotbarSlotUp = true;
	int hotbarSlot = 0;

	if (mousewheel > 0.0f)
	{
		usingNum = false;
		m_inventory.NavigateHotbar(hotbarSlotDown, usingNum, 0);
	}
	else if (mousewheel < 0.0f)
	{
		usingNum = false;
		m_inventory.NavigateHotbar(hotbarSlotUp, usingNum, 0);
	}

	usingNum = true; // So I dont have to put this in every if statement for 1-9

	if (KeyPressedOnce(Key::NUM_1, keyUp1))
	{
		m_inventory.NavigateHotbar(0, usingNum, 1);
	}
	if (KeyPressedOnce(Key::NUM_2, keyUp2))
	{
		m_inventory.NavigateHotbar(0, usingNum, 2);
	}
	if (KeyPressedOnce(Key::NUM_3, keyUp3))
	{
		m_inventory.NavigateHotbar(0, usingNum, 3);
	}
	if (KeyPressedOnce(Key::NUM_4, keyUp4))
	{
		m_inventory.NavigateHotbar(0, usingNum, 4);
	}
	if (KeyPressedOnce(Key::NUM_5, keyUp5))
	{
		m_inventory.NavigateHotbar(0, usingNum, 5);
	}
	if (KeyPressedOnce(Key::NUM_6, keyUp6))
	{
		m_inventory.NavigateHotbar(0, usingNum, 6);
	}
	if (KeyPressedOnce(Key::NUM_7, keyUp7))
	{
		m_inventory.NavigateHotbar(0, usingNum, 7);
	}
	if (KeyPressedOnce(Key::NUM_8, keyUp8))
	{
		m_inventory.NavigateHotbar(0, usingNum, 8);
	}
	if (KeyPressedOnce(Key::NUM_9, keyUp9))
	{
		m_inventory.NavigateHotbar(0, usingNum, 9);
	}
	#pragma endregion

	// Drop Item
	if (input.GetKeyboard().GetKey(Key::Q))
	{
		if (now >= dropItemTimer)
		{
			m_inventory.SetSlotType(m_inventory.selectedSlot, Block::AIR);
			dropItemTimer = now + DROP_ITEM_INTERVAL;
		}
	}
	else
	{
		dropItemTimer = now;
	}

	#pragma region Creative
	// Change selected hotbar item with middleMouseUp to blocktype that player is looking at
	if (m_player.inCreative)
	{
		if (MousePressedOnce(MouseButtons::MIDDLE, middleMouseUp))
		{
			if (!m_player.isLookingAtBlock) return;

			int x = m_player.lookingAtBlockPos.x;
			int y = m_player.lookingAtBlockPos.y;
			int z = m_player.lookingAtBlockPos.z;

			Block* b = m_world.GetBlock(x, y, z);

			m_inventory.SetSlotType(m_inventory.selectedSlot, b->type);
		}
	}

	// Toggle Flying
	if (KeyPressedOnce(Key::SPACE, keyUpSpace) && m_player.inCreative)
	{
		auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastJumpPress).count();

		if (delta <= 500)
		{
			m_player.isFlying = !m_player.isFlying;
			
			if (m_player.isFlying)
			{
				m_player.gravity = 0.0f;
				m_player.velocity.y = 0.0f;
			}
			else
			{
				m_player.gravity = -20.0f * m_player.scaleGravity;
			}

			// Reset so that the next toggle requires double pressing Space again
			lastJumpPress = {};
		}
		else
		{
			lastJumpPress = now;
		}
	}
	#pragma endregion
}

bool Game::KeyPressedOnce(Key key, bool& keyUp)
{
	bool keyDown = input->GetKeyboard().GetKey(key);
	bool pressed = keyDown && !keyUp;
	keyUp = keyDown;
	return pressed;
}

bool Game::MousePressedOnce(MouseButtons mouseButton, bool& mouseButtonUp)
{
	bool mouseButtonDown = input->GetMouse().GetButtonDown(mouseButton);
	bool pressed = mouseButtonDown && !mouseButtonUp;
	mouseButtonUp = mouseButtonDown;
	return pressed;
}

void Game::LockMouseWindows()
{
	#ifdef WINDOWS_BUILD
	// Lock/unlock mouse depending on bool mouseLocked
	if (m_player.mouseLocked)
	{
		graphics->LockMouse();
	}
	else
	{
		graphics->UnlockMouse();
	}
	#endif
}

void Game::InitializeOpenGLES()
{

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRangef(0.0f, 1.0f);
	glClearDepthf(1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void Game::DayNightCycle()
{
	using clock = std::chrono::steady_clock;

	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	static clock::time_point cycleStartTime = now;

	// Calculates how much time has gone by in seconds
	float elapsed = std::chrono::duration<float>(now - cycleStartTime).count();
	float normalizedTime = fmod(elapsed, day_duration) / day_duration;
	
	// Calculate sun angle (does a full rotation per day)
	sunAngle = normalizedTime * glm::two_pi<float>();

	// Move sun around in a vertical circle
	sunDirection.x = 0.0f;
	sunDirection.y = sin(sunAngle);
	sunDirection.z = cos(sunAngle);
	sunDirection = glm::normalize(sunDirection);

	// sunDirection.y =  1 means that dayFactor =  1 which means that its day
	// sunDirection.y = -1 means that dayFactor = -1 which means that its night
	dayFactor = (sunDirection.y + 1.0f) * 0.5f;

	// Night and day colors
	glm::vec3 nightColor(0.13f, 0.18f, 0.27f);
	glm::vec3 dayColor(0.47f, 0.66f, 1.0f);

	// Mix sky color depending on dayFactor between nightColor and dayColor
	skyColor = glm::mix(nightColor, dayColor, dayFactor);
}

void Game::ClearScreen()
{
	glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}