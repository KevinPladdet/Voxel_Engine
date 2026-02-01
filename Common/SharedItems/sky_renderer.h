#pragma once

#include <glm/glm.hpp>
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "shaderClass.h"
#include "texture.h"

class Sky_Renderer
{
public:
    Sky_Renderer() = default;
    ~Sky_Renderer();

    void Init();
    void Render(const glm::vec3& cameraPos, const glm::vec3& sunDirection, 
                float dayFactor, float sunAngle,
                const glm::mat4& view, const glm::mat4& projection);

private:
    // How far away the Sun and Moon are from player
    static constexpr float SKY_DISTANCE = 100.0f;

    VAO m_skyVAO;
    VBO m_skyVBO;
    EBO m_skyEBO;

    Shader* m_skyShader;
    Texture* m_sunTexture;
    Texture* m_moonTexture;
};