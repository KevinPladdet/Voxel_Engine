#pragma once

#include "IncludeGraphics.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Input.h"
#include "IInput.h"

#include "shaderClass.h"
#include "world.h"
#include "audiomanager.h"

class Player
{
public:

	enum PlayerDirection
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		JUMP
	};

	enum PlayerInputs
	{
		LEFT_CLICK
	};

	Player(int width, int height, glm::vec3 position, World& world, AudioManager& audiomanager);
	Player() = default;
	const glm::vec3 GetPosition() const { return playerPos; }

	void Update(Shader& shader, const char* uniform, float deltaTime);
	void Inputs(PlayerDirection direction, float deltaTime, const Input& input);
	void MouseInput(float deltaTime, const Input& input);
	
	void MoveAndCollide(float deltaTime);
	bool Colliding();
	void Acceleration(float deltaTime);
	
	void LookingRaycast();
	
	void DestroyBlock(Block::BlockType type);
	void PlaceBlock(Block::BlockType type);

	void BlockPlaySFX(Block::BlockType type);
	void BlockDestroySFX(Block::BlockType type);
	void BlockPlaceSFX(Block::BlockType type);

	
	bool BlockPlayerAABB(int bx, int by, int bz);
	bool UnderPlayerAABB(float playerX, float playerZ); // Used to check if player falls off block when crouching

	void FootstepsSFX();
	Block::BlockType GetBlockBelow();

	void SetCreative();
	void SetSurvival();

	glm::vec3 playerPos;
	glm::vec3 velocity = glm::vec3(0.0f);

	// Direction of player depending on what movement inputs are pressed
	glm::vec3 currentDir = glm::vec3(0.0f);

	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

	// Used for mvp matrix (model view projection)
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	// Window size
	int width;
	int height;

	// In real Minecraft the player is 0.6 wide, 1.8 tall and eye height is at 1.62
	float playerWidth = 0.3f;
	float playerHeight = 1.8f;
	float eyeHeight = 1.62f;
	float crouchingEyeHeight = 1.27f;
	float currentEyeHeight = eyeHeight;

	// Moving
	float currentSpeed = 5.0f;
	float walkingSpeed = 5.0f;
	float sprintingSpeed = walkingSpeed * 1.3f;
	float crouchingSpeed = walkingSpeed * 0.3f;
	bool isSprinting = false;
	bool isCrouching = false;
	bool holdingMoveInput = false; // is true whenever player is holding any of W A S D

	// Flying
	float flyingSpeed = walkingSpeed * 2.5f;
	float flyingSprintingSpeed = walkingSpeed * 5.0f;

	// Jumping
	float scaleGravity = 1.5f;
	float jumpForce = 7.1f * sqrt(scaleGravity);
	float gravity = -20.0f * scaleGravity;
	bool isGrounded = false;
	bool isJumping = false;
	bool hitHead = false;
	
	// FOV
	float currentFOV = 70.0f;
	float baseFOV = 70.0f;
	float sprintFOV = 1.2f; // +20% multiplier

	// Looking Variables
	float nearPlane = 0.1f;
	float farPlane = 250.0f;
	float reach = 4.5f; // From how far the player can place and destroy blocks
	bool isLookingAtBlock = false;
	// Block position that player is looking at
	glm::ivec3 lookingAtBlockPos = glm::ivec3(0);
	// Will place blocks in front of the face of the block the player is looking at
	glm::ivec3 placeBlockPos = glm::ivec3(0);

	// Mouse Variables
	bool mouseLocked = true;
	float sensitivity = 200.0f;
	float yaw = -90.0f;
	float pitch = 0.0f;
	float lastX = 1920.0f / 2.0f;
	float lastY = 1080.0f / 2.0f;

	// Walking SFX
	float footstepTimer = 0.0f;
	float walkInterval = 0.4f; // Every 0.4s it will play walking SFX
	float sprintInterval = 0.3f; // Every 0.3s it will play walking SFX

	// Creative
	bool inCreative = false;
	bool isFlying = false;
private:
	World* m_world = nullptr;
	AudioManager* m_audiomanager = nullptr;
};