#pragma once
#include "glm/glm.hpp"
#include <glbinding/gl/gl.h>
#include "Texture.hpp"

using namespace gl;

enum MaterialTextureBitsetIndex : unsigned int
{
    MAT_COLOR_BIT,
    MAT_ROUGHNESS_BIT,
    MAT_METALLIC_BIT,
    MAT_NORMAL_BIT,
    MAT_AO_BIT
};

/**
 * @brief Struct containing PBR material information passed to the shader
 */
class Material
{
public:
    /**
     * @brief Initializes the material struct
     * @param color The color of the material (clamped to positive values)
     * @param roughness The roughness of the material (clamped to [0,1])
     * @param metallic The metalness of the material (clamped to [0,1])
     * @param ior Index of Refraction (clamped to positive values)
     */
    Material(glm::vec4 color = glm::vec4(1.0f), float roughness = 1.0f, float metallic = 0.0f,
        float ior = 1.5f);

    /**
     * @brief Sets the color of the material (clamped to positive values if vec3)
     */
    void      setColor(std::variant<glm::vec4, std::shared_ptr<Texture>> color);
    glm::vec4 getColor() const;

    /**
    * @brief Sets the normal map of the material
    */
    void      setNormalMap(const std::shared_ptr<Texture>& normalMap);

    /**
    * @brief Sets the ambient occlusion texture of the material
    */
    void      setAoMap(const std::shared_ptr<Texture>& aoMap);

    /**
     * @brief Sets the roughness of the material (clamped to [0,1] if float)
     */
    void  setRoughness(std::variant<float, std::shared_ptr<Texture>> roughness);
    float getRoughness() const;

    /**
     * @brief Sets the metalness of the material (clamped to [0,1] if float)
     */
    void  setMetallic(std::variant<float, std::shared_ptr<Texture>> metallic);
    float getMetallic() const;

    /**
     * @brief Sets the index of refraction (clamped to positive values)
     */
    void  setIOR(float ior);
    float getIOR() const;

    /**
    * @brief Draws a ImGui-window containing the material parameters.
    * @return true if the parameters were changed.
    */
    bool drawGuiWindow();

    /**
    * @brief Draws the ImGui-content containing the material parameters.
    * @return true if the parameters were changed.
    */
    bool drawGuiContent();

private:
    //uvec2 can be packed vec4 or bindless sampler2D
    glm::uvec2 m_albedo{ 0, 0 };
    glm::uvec2 m_roughness{ 0, 0 };
    glm::uvec2 m_metallic{ 0, 0 };

    float m_ior = 1.0f;
    GLuint m_isTextureBitset = 0u; // texture if bit=1, color otherwise

    GLuint64 m_normal = 0;
    GLuint64 m_ao = 0;

};
