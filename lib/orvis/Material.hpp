#pragma once
#include <cstdint>
#include "glm/glm.hpp"
#include "Createable.hpp"

/**
 * @brief Struct containing PBR material information passed to the shader
 */
class Material : Createable<Material>
{
public:
    /**
     * @brief Initializes the material struct
     * @param color The color of the material (clamped to positive values)
     * @param roughness The roughness of the material (clamped to [0,1])
     * @param metalness The metalness of the material (clamped to [0,1])
     * @param ior Index of Refraction (clamped to positive values)
     */
    Material(glm::vec3 color = glm::vec3(1.0f), float roughness = 0.05f, float metalness = 0.0f,
             float ior = 1.5f);

    /**
     * @brief Sets the color of the material (clamped to positive values)
     */
    void      setColor(glm::vec3 color);
    glm::vec3 getColor() const;

    /**
     * @brief Sets the roughness of the material (clamped to [0,1])
     */
    void  setRoughness(float roughness);
    float getRoughness() const;

    /**
     * @brief Sets the metalness of the material (clamped to [0,1])
     */
    void  setMetalness(float metalness);
    float getMetalness() const;

    /**
     * @brief Sets the index of refraction (clamped to positive values)
     */
    void  setIOR(float ior);
    float getIOR() const;

private:
    uint32_t m_data1 = 0; //col.rg
    uint32_t m_data2 = 0; //col.b, roughness
    uint32_t m_data3 = 0; //metalness, ior
    
};
