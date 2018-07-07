#pragma once
#include "Creatable.hpp"
#include "glm/glm.hpp"

/**
 * @brief Struct containing lighting information passed to the shader
 * Currently only point lights!
 */
struct Light : Creatable<Light>
{
    /**
     * @brief Initializes the light struct
     * @param position The position of the light source
     * @param color The color of the light source (clamped to positive values)
     */
    Light(glm::vec3 position = glm::vec3(1.0f), glm::vec3 color = glm::vec3(1.0f));

    glm::vec3 getPosition() const;
    void      setPosition(glm::vec3 position);

    /**
     * @brief Sets the color of the light source (clamped to positive values)
     */
    void      setColor(glm::vec3 color);
    glm::vec3 getColor() const;

private:
    uint32_t   m_data1            = 0; //pos.xy
    uint32_t   m_data2            = 0; //pos.z, col.r
    uint32_t   m_data3            = 0; //col.gb
};
