#include "sky_renderer.h"
#include <glm/gtc/type_ptr.hpp>

static GLfloat quadVertices[] =
{
	// Positions         // UVs
	-1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // Top left of quad
	 1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // Bottom left of quad
	 1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // Bottom right of quad
	-1.0f,  1.0f, 0.0f,  0.0f, 1.0f, // Top right of quad
};

static GLuint quadIndices[] =
{ 
	0, 1, 2, 
	2, 3, 0 
};

Sky_Renderer::~Sky_Renderer()
{
    m_skyVAO.Delete();
    m_skyVBO.Delete();
    m_skyEBO.Delete();

    if (m_sunTexture)
    {
        m_sunTexture->Delete();
        delete m_sunTexture;
    }
    if (m_moonTexture)
    {
        m_moonTexture->Delete();
        delete m_moonTexture;
    }
    if (m_skyShader)
    {
        m_skyShader->Delete();
        delete m_skyShader;
    }
}

void Sky_Renderer::Init()
{
    // Sky Shader
    m_skyShader = new Shader("../Common/SharedItems/Assets/Shaders/sky_quad.vert", "../Common/SharedItems/Assets/Shaders/sky_quad.frag");
    // Textures for Sun and Moon
    m_sunTexture = new Texture("../Common/SharedItems/Assets/Textures/Sun.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    m_moonTexture = new Texture("../Common/SharedItems/Assets/Textures/Moon.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);

    m_sunTexture->TexUnit(*m_skyShader, "tex0", 0);
    m_moonTexture->TexUnit(*m_skyShader, "tex0", 0);

    // Sky Quad
    m_skyVAO.Bind();
    m_skyVBO = VBO(quadVertices, sizeof(quadVertices));
    m_skyEBO = EBO((GLuint*)quadIndices, sizeof(quadIndices));
    m_skyVAO.LinkAttrib(m_skyVBO, 0, 3, GL_FLOAT, 5 * sizeof(float), (void*)0);
    m_skyVAO.LinkAttrib(m_skyVBO, 1, 2, GL_FLOAT, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    m_skyVAO.Unbind();
}

void Sky_Renderer::Render(const glm::vec3& cameraPos, const glm::vec3& sunDirection, float dayFactor, float sunAngle, const glm::mat4& view, const glm::mat4& projection)
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    // Makes the background transparent by blending the colors for also a glowlike effect
    glBlendFunc(GL_ONE, GL_ONE);

    m_skyShader->Activate();
    
    // Remove translation from view matrix so sky stays centered with camera
    glm::mat4 viewRotationOnly = glm::mat4(glm::mat3(view));
    glm::mat4 camMatrix = projection * viewRotationOnly;
    glUniformMatrix4fv(glGetUniformLocation(m_skyShader->ID, "camMatrix"), 1, GL_FALSE, glm::value_ptr(camMatrix));
    m_skyVAO.Bind();

    // Size of Sun & Moon
    float size = 40.0f;


    // Sun
    glm::vec3 sunPos = sunDirection * SKY_DISTANCE;
    glm::mat4 model = glm::translate(glm::mat4(1.0f), sunPos);
    // Billboarding: Rotate quad around X-axis to face towards player
    model = glm::rotate(model, glm::pi<float>() - sunAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(size));

    glUniformMatrix4fv(glGetUniformLocation(m_skyShader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    m_sunTexture->Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    

    // Moon (Opposite of Sun)
    glm::vec3 moonPos = -sunDirection * SKY_DISTANCE;
    model = glm::translate(glm::mat4(1.0f), moonPos);
    // Billboarding: Make the quad face the same direction as the player
    model = glm::rotate(model, -sunAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(size));

    glUniformMatrix4fv(glGetUniformLocation(m_skyShader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    m_moonTexture->Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    
    m_skyVAO.Unbind();

    // Makes the background transparent by blending the colors for also a glowlike effect
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}