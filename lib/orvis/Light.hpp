#pragma once
#include "glm/glm.hpp"
#include <memory>
#include "Texture.hpp"
#include "FrameBuffer.hpp"
#include "Shader.hpp"
#include "Scene.hpp"

// forward declaration
class Scene;

enum class LightType : int
{
    directional = 0,
    point = 1,
    spot = 2
};

/**
 * @brief Struct containing lighting information passed to the shader.
 * Currently only point lights!
 */
class Light
{
public:

    /** @brief Standard constructor. Only for deafult-constructability. Use factory functions (make...()) to create a light. */
    Light() = default;

    /** 
     * @brief Copy constructor. Copies all values to the new light object. 
     * Caution: Shadow map texture object is not copied (only its GPU-handle)!
     */
    Light(const Light& other);

    /** 
     * @brief Copy assignment operator. Copies all values to the new light object. 
     * Caution: Shadow map texture object is not copied (only its GPU-handle)!
     */
    Light& operator=(const Light& other);

    /**
     * @brief Initializes a point light
     * @param position The position of the light source
     * @param color The color of the light source (clamped to positive values)
     */
    static std::shared_ptr<Light> makePointLight(glm::vec3 position = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 color = glm::vec3(1.0f));

    /**
    * @brief Initializes a directional light
    * @param direction The direction of the light source
    * @param color The color of the light source (clamped to positive values)
    */
    static std::shared_ptr<Light> makeDirectionalLight(glm::vec3 direction = glm::normalize(glm::vec3(0.5f, -1.0f, -0.5f)), glm::vec3 color = glm::vec3(1.0f));

    /**
    * @brief Initializes a directional light
    * @param position The position of the light source
    * @param direction The direction of the light source
    * @param color The color of the light source (clamped to positive values)
    * @param cutOff The cut off of the light source (clamped to positive values)
    */
    static std::shared_ptr<Light> makeSpotLight(glm::vec3 position = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 direction = glm::normalize(glm::vec3(0.5f, -1.0f, -0.5f)), glm::vec3 color = glm::vec3(1.0f), float cutOff = glm::radians(25.0f));

    /**
     * @brief Updates the light space matrix and renders the shadow map
     * @param scene The scene that is rendered into the shadow map
     */
    void update(const std::shared_ptr<Scene>& scene);

    /**
    * @brief Draws a ImGui-window containing the light parameters.
    * @return true if the parameters were changed.
    */
    bool drawGuiWindow();

    /**
    * @brief Draws the ImGui-content containing the light parameters.
    * @return true if the parameters were changed.
    */
    bool drawGuiContent();

    glm::vec3 color = glm::vec3(1.0f);                                      // all
    float cutOff = glm::radians(25.0f);                                     // spot
    glm::vec3 position = glm::vec3(0.0f, 1.0f, 0.0f);                       // spot, point    
    int pcfKernelSize = 1;                                                  // all (used for SM filtering)
    glm::vec3 direction = glm::normalize(glm::vec3(0.5f, -1.0f, -0.5f));    // dir, spot  

private:
    struct ShadowMap
    {
        ShadowMap();

        void render(const std::shared_ptr<Scene>& scene) const;

        Texture shadowTexture{ GL_TEXTURE_2D, GL_DEPTH_COMPONENT32F, glm::ivec2(1024, 1024) };
        FrameBuffer shadowFBO{ shadowTexture.getSize() };
        Program shadowProgram;
    };

    Light(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float cutOff, LightType type);

    void recalculateLightSpaceMatrix(const std::shared_ptr<Scene>& scene);

    LightType m_type = LightType::point; // 0 directional, 1 point light, 2 spot light

    //shadow mapping stuff
    glm::mat4 m_lightSpaceMatrix = glm::mat4(1.0f);
    GLuint64 m_shadowMapHandle = 0; // can be sampler2DShadow or samplerCubeShadow
    std::unique_ptr<ShadowMap> m_shadowMap; // works as padding in glsl (is 64bit)
};
