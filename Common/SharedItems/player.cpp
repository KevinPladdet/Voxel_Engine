#include "player.h"

Player::Player(int width, int height, glm::vec3 position, World& world, AudioManager& audiomanager)
{
	Player::width = width;
	Player::height = height;
	playerPos = position;
	m_world = &world;
	m_audiomanager = &audiomanager;
}

void Player::Update(Shader& shader, const char* uniform, float deltaTime)
{
	// Change FOV depending on crouching / walking / running
	float FOV = baseFOV;
	if (isCrouching && !isFlying)
	{
		FOV = baseFOV;
	}
	else
	{
		if (isSprinting)
		{
			FOV = isSprinting ? baseFOV * sprintFOV : baseFOV;
		}
	}

	// Increase FOV by 10% if flying
	if (isFlying)
	{
		FOV *= 1.1f;
	}

	// Update FOV gradually over time
	float fovChangeSpeed = 10.0f;
	currentFOV += (FOV - currentFOV) * fovChangeSpeed * deltaTime;

	// Update eyeHeight gradually over time
	float targetEyeHeight = isCrouching ? crouchingEyeHeight : eyeHeight;
	float eyeChangeSpeed = 10.0f;
	currentEyeHeight += (targetEyeHeight - currentEyeHeight) * eyeChangeSpeed * deltaTime;

	// If not grounded, start falling
	if (!isGrounded && !isFlying)
	{
		velocity.y += gravity * deltaTime;
	}

	// When flying, decelerate Y velocity if not pressing up/down
	if (isFlying)
	{
		if (isCrouching)
		{
			velocity.y = -jumpForce;
		}
		else
		{
			float flyDeceleration = 10.0f;
			velocity.y *= std::exp(-flyDeceleration * deltaTime);

			if (std::abs(velocity.y) < 0.05f)
			{
				velocity.y = 0.0f;
			}
		}
	}

	Acceleration(deltaTime);

	MoveAndCollide(deltaTime);

	// Walking SFX
	if (isGrounded && holdingMoveInput && !isFlying)
	{
		footstepTimer -= deltaTime;

		if (footstepTimer <= 0.0f)
		{
			FootstepsSFX();

			// Reset timer
			footstepTimer = isSprinting ? sprintInterval : walkInterval;
		}
	}
	else
	{
		// Reset timer when not walking so first step plays immediately
		footstepTimer = 0.0f;
	}

	LookingRaycast();
	
	// Position of camera at eye height
	glm::vec3 cameraPos = playerPos + glm::vec3(0.0f, currentEyeHeight, 0.0f);
	
	// View and projection (used for mvp matrix)
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, Up);
	projection = glm::perspective(glm::radians(currentFOV), (float)width / (float)height, nearPlane, farPlane);

	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(projection * view));
}

void Player::Inputs(PlayerDirection direction, float deltaTime, const Input& input)
{
	glm::vec3 forwardXZ = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
	glm::vec3 rightXZ = glm::normalize(glm::cross(forwardXZ, Up));

	if (isCrouching && !isFlying)
	{
		currentSpeed = crouchingSpeed;
	}
	else
	{
		if (isSprinting)
		{
			if (isFlying)
			{
				currentSpeed = flyingSprintingSpeed;
			}
			else
			{
				currentSpeed = sprintingSpeed;
			}
		}
		else
		{
			if (isFlying)
			{
				currentSpeed = flyingSpeed;
			}
			else
			{
				currentSpeed = walkingSpeed;
			}
		}
	}

	switch (direction)
	{
	// Directional Movement
	case FORWARD:
		currentDir += forwardXZ;
		break;
	case BACKWARD:
		currentDir -= forwardXZ;
		break;
	case LEFT:
		currentDir -= rightXZ;
		break;
	case RIGHT:
		currentDir += rightXZ;
		break;

	// Jump
	case JUMP:
		if (isFlying)
		{
			velocity.y = jumpForce;
		}
		else if (isGrounded)
		{
			velocity.y = jumpForce;
			isGrounded = false;
		}
		break;
	}
}

void Player::MouseInput(float deltaTime, const Input& input)
{
	// Store the coordinates of the cursor
	float mouseX = input.GetMouse().GetPosition().x;
	float mouseY = input.GetMouse().GetPosition().y;

	// Creating delta positions instead of absolute positions
	float xoffset = mouseX - lastX;
	float yoffset = lastY - mouseY;
	lastX = mouseX;
	lastY = mouseY;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void Player::MoveAndCollide(float deltaTime)
{
	bool wasGrounded = isGrounded;
	hitHead = false;

	// X axis
	playerPos.x += velocity.x * deltaTime;
	if (Colliding()) // Prevent movement
	{
		playerPos.x -= velocity.x * deltaTime;
		velocity.x = 0.0f;
	}
	else if (isCrouching && isGrounded && UnderPlayerAABB(playerPos.x, playerPos.z))
	{ // Prevent movement when falling of a block while crouching
		playerPos.x -= velocity.x * deltaTime;
		velocity.x = 0.0f;
	}

	// Y axis
	playerPos.y += velocity.y * deltaTime;
	if (Colliding())
	{
		playerPos.y -= velocity.y * deltaTime;

		// If moving up and collided, hitHead = true
		if (velocity.y > 0.0f)
		{
			hitHead = true;
		}

		// If moving down and collided, isGrounded = true
		if (velocity.y < 0.0f)
		{
			isGrounded = true;
		}

		velocity.y = 0.0f;
	}

	// Z axis
	playerPos.z += velocity.z * deltaTime;
	if (Colliding())
	{
		playerPos.z -= velocity.z * deltaTime;
		velocity.z = 0.0f;
	}
	else if (isCrouching && isGrounded && UnderPlayerAABB(playerPos.x, playerPos.z))
	{ // Prevent movement when falling of a block while crouching
		playerPos.z -= velocity.z * deltaTime;
		velocity.z = 0.0f;
	}

	// Check if there is ground below player when velocity.y is 0
	if (velocity.y <= 0.0f)
	{
		float distance = 0.05f;
		playerPos.y -= distance;

		if (Colliding())
		{
			isGrounded = true;
		}
		else
		{
			isGrounded = false;
		}	

		playerPos.y += distance;
	}
	else // If velocity.y is going upwards, set isGrounded to false
	{
		isGrounded = false;
	}
	
	if (!wasGrounded && isGrounded && !holdingMoveInput)
	{	
		// Kill horizontal momentum upon landing
		velocity.x *= 0.4f;
		velocity.z *= 0.4f;

		// Set velocity to 0 when it is super slow
		if (std::abs(velocity.x) < 0.05f)
		{
			velocity.x = 0.0f;
		}
		if (std::abs(velocity.z) < 0.05f)
		{
			velocity.z = 0.0f;
		}
	}
}

bool Player::Colliding()
{
	int minX = (int)(floor(playerPos.x - playerWidth));
	int maxX = (int)(floor(playerPos.x + playerWidth));

	int minY = (int)(floor(playerPos.y));
	int maxY = (int)(floor(playerPos.y + playerHeight));

	int minZ = (int)(floor(playerPos.z - playerWidth));
	int maxZ = (int)(floor(playerPos.z + playerWidth));

	for (int x = minX; x <= maxX; x++)
	{
		for (int y = minY; y <= maxY; y++)
		{
			for (int z = minZ; z <= maxZ; z++)
			{
				Block* b = m_world->GetBlock(x, y, z);
				if (b && b->type != Block::AIR)
				{
					return true;
				}
			}
		}
	}	

	return false;
}

void Player::Acceleration(float deltaTime)
{
	glm::vec3 dir = currentDir;
	dir.y = 0.0f;

	bool isMoving = false;
	if (glm::length(dir) > 0.0f)
	{
		isMoving = true;
		dir = glm::normalize(dir);
	}

	glm::vec3 velocityXZ(velocity.x, 0.0f, velocity.z);
	
	float groundAcceleration = 20.0f;
	float groundDeceleration = 50.0f;
	float airAcceleration = 8.0f;
	float airDeceleration = 1.0f;

	if (isMoving)
	{
		glm::vec3 targetVelocity = dir * currentSpeed;

		// Boost velocity by 5% when in the air and sprinting
		if (!isGrounded && isSprinting)
		{
			targetVelocity *= 1.05f;
		}
		
		float acceleration = isGrounded ? groundAcceleration : airAcceleration;

		velocityXZ += (targetVelocity - velocityXZ) * acceleration * deltaTime;
	}
	else
	{
		float deceleration = isGrounded ? groundDeceleration : airDeceleration;

		velocityXZ *= std::exp(-deceleration * deltaTime);

		// Set velocity to 0 when it is super slow
		if (glm::length(velocityXZ) < 0.05f)
		{
			velocityXZ = glm::vec3(0.0f);
		}
	}

	velocity.x = velocityXZ.x;
	velocity.z = velocityXZ.z;

	currentDir = glm::vec3(0.0f);
}

void Player::LookingRaycast()
{
	isLookingAtBlock = false;

	// currentPos is player camera and dir is where the player is currently looking
	glm::vec3 currentPos = playerPos + glm::vec3(0.0f, currentEyeHeight, 0.0f);
	glm::vec3 dir = glm::normalize(cameraFront);

	// Raycast will check for blocks every 0.02f
	float step = 0.02f;

	// change currentPos into block coordinates
	// lastAirBlockPos is used for the air block position right before hitting a block
	glm::ivec3 lastAirBlockPos((int)std::floor(currentPos.x), (int)std::floor(currentPos.y), (int)std::floor(currentPos.z));
	
	glm::ivec3 currentBlockPos = lastAirBlockPos;

	for (float rayDistance = 0.0f; rayDistance <= reach; rayDistance += step)
	{
		glm::vec3 rayPos = currentPos + dir * rayDistance;

		glm::ivec3 blockPos((int)std::floor(rayPos.x), (int)std::floor(rayPos.y), (int)std::floor(rayPos.z));

		// Always continue to the next iteration unless it enters a new block position
		// Continue if the blockPos of the ray isn't a new block position
		if (blockPos == currentBlockPos)
		{
			continue;
		}

		// Set lastAirBlockPos
		lastAirBlockPos = currentBlockPos;
		currentBlockPos = blockPos;

		Block* b = m_world->GetBlock(blockPos.x, blockPos.y, blockPos.z);
		if (b && b->type != Block::AIR && b->type != Block::WATER && b->type != Block::LAVA)
		{
			isLookingAtBlock = true;
			lookingAtBlockPos = blockPos;
			placeBlockPos = lastAirBlockPos;
			return;
		}
	}
}

void Player::DestroyBlock(Block::BlockType type)
{
	if (!isLookingAtBlock) return;

	int x = lookingAtBlockPos.x;
	int y = lookingAtBlockPos.y;
	int z = lookingAtBlockPos.z;

	Block* b = m_world->GetBlock(x, y, z);

	if (!b) return;
	
	// Can break bedrock when in creative, but not in survival
	if (!inCreative)
	{
		if (b->type == Block::BEDROCK && type == Block::AIR) return;
	}
	
	// Can't break water
	if (b->type == Block::WATER && type == Block::AIR) return;

	// Play SFX for block that got placed
	Block::BlockType destroyedType = b->type;
	if (destroyedType == Block::GLASS)
	{
		BlockDestroySFX(destroyedType);
	}
	else
	{
		BlockPlaySFX(destroyedType);
	}

	m_world->SetBlock(x, y, z, type);

	isLookingAtBlock = false;
}

void Player::PlaceBlock(Block::BlockType type)
{
	if (!isLookingAtBlock) return;

	// Prevent being able to place air
	if (type == Block::AIR) return;

	int x = placeBlockPos.x;
	int y = placeBlockPos.y;
	int z = placeBlockPos.z;

	Block* b = m_world->GetBlock(x, y, z);

	if (!b) return;
	if (b->type != Block::AIR && b->type != Block::WATER && b->type != Block::LAVA) return;

	if (BlockPlayerAABB(x, y, z)) return;

	m_world->SetBlock(x, y, z, type);
	
	// Play SFX for block that got placed
	if (type == Block::GLASS)
	{
		BlockPlaceSFX(type);
	}
	else
	{
		BlockPlaySFX(type);
	}

	isLookingAtBlock = false;
}

void Player::BlockPlaySFX(Block::BlockType type)
{
	switch (type)
	{
		case Block::BEDROCK:	m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Block/Block_Bedrock.mp3", 1.0f);	 break;
		case Block::DIRT:		m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Block/Block_Dirt.mp3", 1.0f);	     break;
		case Block::GRASS:		m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Block/Block_Grass.mp3", 0.4f);	 break;
		case Block::SAND:		m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Block/Block_Sand.mp3", 1.0f);	     break;
		case Block::STONE:		m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Block/Block_Stone.mp3", 1.0f);	 break;
		case Block::WATER:		m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Block/Block_Water.mp3", 1.0f);	 break;
		case Block::LAVA:		m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Block/Block_Lava.mp3", 1.0f);      break;
		case Block::OAK_LOG:    m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Block/Block_OakLog.mp3", 1.0f);    break;
		case Block::OAK_PLANKS: m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Block/Block_OakPlanks.mp3", 1.0f); break;
		default:				m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Block/Block_Stone_SFX.mp3", 1.0f); break;
	}
}

void Player::BlockDestroySFX(Block::BlockType type)
{
	switch (type)
	{
		case Block::GLASS: m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Block/Glass_Break.mp3", 1.0f); break;
	}
}

void Player::BlockPlaceSFX(Block::BlockType type)
{
	switch (type)
	{
		case Block::GLASS: m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Block/Glass_Place.mp3", 1.0f); break;
	}
}

bool Player::BlockPlayerAABB(int bx, int by, int bz)
{
	float blockSize = 1.0f;

	return(
		// X axis
		playerPos.x - playerWidth < bx + blockSize &&
		playerPos.x + playerWidth > bx &&

		// Y axis
		playerPos.y < by + blockSize &&
		playerPos.y + playerHeight > by &&

		// Z axis
		playerPos.z - playerWidth < bz + blockSize &&
		playerPos.z + playerWidth > bz);
}

bool Player::UnderPlayerAABB(float playerX, float playerZ)
{
	// Bottom of playerPos of each corner
	float corners[4][2] = 
	{
		{playerX - playerWidth, playerZ - playerWidth},
		{playerX + playerWidth, playerZ - playerWidth},
		{playerX - playerWidth, playerZ + playerWidth},
		{playerX + playerWidth, playerZ + playerWidth}
	};

	// Using floor so it checks exactly what block is under the player position
	int groundY = (int)floor(playerPos.y - 0.1f);
	
	// Checking all 4 corners to look for blocks under it
	for (int i = 0; i < 4; i++)
	{
		int checkX = (int)floor(corners[i][0]);
		int checkZ = (int)floor(corners[i][1]);

		Block* b = m_world->GetBlock(checkX, groundY, checkZ);

		if (b && b->type != Block::AIR)
		{
			return false;  // Found blocks under player, so velocity won't happen
		}
	}

	return true; // Didn't find any blocks under player, velocity will happen
}

// Plays a SFX depending on what block type player is walking on
void Player::FootstepsSFX()
{
	// Only play footsteps when grounded and actually moving
	// Only play walking SFX if player is grounded and moving
	if (!isGrounded || !holdingMoveInput || isFlying)
	{
		return;
	}
	
	// Checks if player is actually moving
	float horizontalSpeed = glm::length(glm::vec2(velocity.x, velocity.z));
	if (horizontalSpeed < 0.5f)
	{
		return;
	}
	
	Block::BlockType blockBelow = GetBlockBelow();
	if (blockBelow == Block::AIR)
	{
		return;
	}

	// Play walking SFX depending on what blocktype is below player
	switch (blockBelow)
	{
		case Block::BEDROCK:	m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Walk/Walk_Bedrock.mp3", 0.5f);		break;
		case Block::DIRT:		m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Walk/Walk_Dirt.mp3", 0.5f);			break;
		case Block::GRASS:		m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Walk/Walk_Grass.mp3", 0.5f);			break;
		case Block::SAND:		m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Walk/Walk_Sand.mp3", 0.5f);			break;
		case Block::STONE:		m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Walk/Walk_Stone.mp3", 0.5f);			break;
		case Block::OAK_LOG:    m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Walk/Walk_OakLog.mp3", 0.5f);			break;
		case Block::OAK_PLANKS: m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Walk/Walk_OakPlanks.mp3", 0.5f);		break;
		
		default:				m_audiomanager->PlaySFX("../Common/SharedItems/Assets/Sfx/Walk/Walk_Stone.mp3", 0.5f);     break;
	}
}

// Returns blocktype of block player is standing on
Block::BlockType Player::GetBlockBelow()
{
	int groundY = (int)floor(playerPos.y - 0.1f);
	int playerBlockX = (int)floor(playerPos.x);
	int playerBlockZ = (int)floor(playerPos.z);

	Block* b = m_world->GetBlock(playerBlockX, groundY, playerBlockZ);
	
	if (b && b->type != Block::AIR)
	{
		return b->type;
	}

	return Block::AIR;
}

void Player::SetCreative()
{
	printf("SetCreative()\n");
	inCreative = true;
	reach = 5.0f;
}

void Player::SetSurvival()
{
	printf("SetSurvival()\n");
	inCreative = false;
	reach = 4.5f;

	isFlying = false;
	gravity = -20.0f * scaleGravity;
}