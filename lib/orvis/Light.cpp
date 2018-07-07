#include "Light.hpp"

Light::Light(glm::vec3 position, glm::vec3 color)
{
    color   = glm::max(glm::vec3(0.0f), color);
    m_data1 = glm::packHalf2x16(glm::vec2(position.x, position.y));
    m_data2 = glm::packHalf2x16(glm::vec2(position.z, color.r));
    m_data3 = glm::packHalf2x16(glm::vec2(color.g, color.b));
}

glm::vec3 Light::getPosition() const
{
    return glm::vec3(glm::unpackHalf2x16(m_data1), glm::unpackHalf2x16(m_data2).x);
}

void Light::setPosition(glm::vec3 position)
{
    const float colR   = glm::unpackHalf2x16(m_data2).y;
    m_data1            = glm::packHalf2x16(glm::vec2(position.x, position.y));
    m_data2            = glm::packHalf2x16(glm::vec2(position.z, colR));
}

glm::vec3 Light::getColor() const
{
    return glm::vec3(glm::unpackHalf2x16(m_data2).y, glm::unpackHalf2x16(m_data3));
}

void Light::setColor(glm::vec3 color)
{
    color            = glm::max(glm::vec3(0.0f), color);
    const float posZ = glm::unpackHalf2x16(m_data2).x;
    m_data2          = glm::packHalf2x16(glm::vec2(posZ, color.r));
    m_data3          = glm::packHalf2x16(glm::vec2(color.g, color.b));
}
