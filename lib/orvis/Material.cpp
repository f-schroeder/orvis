#include "Material.hpp"

Material::Material(glm::vec3 color, float roughness, float metalness, float ior)
{
    color     = glm::max(glm::vec3(0.0f), color);
    roughness = glm::clamp(roughness, 0.0f, 1.0f);
    metalness = glm::clamp(metalness, 0.0f, 1.0f);
    ior       = glm::max(0.0f, ior);
    m_data1 = glm::packHalf2x16(glm::vec2(color.x, color.y));
    m_data2 = glm::packHalf2x16(glm::vec2(color.z, roughness));
    m_data3 = glm::packHalf2x16(glm::vec2(metalness, ior));
}

glm::vec3 Material::getColor() const
{
    return glm::vec3(glm::unpackHalf2x16(m_data1), glm::unpackHalf2x16(m_data2).x);
}

void Material::setColor(glm::vec3 color)
{
    color                 = glm::max(glm::vec3(0.0f), color);
    const float roughness = getRoughness();
    m_data1 = glm::packHalf2x16(glm::vec2(color.x, color.y));
    m_data2 = glm::packHalf2x16(glm::vec2(color.z, roughness));
}

float Material::getRoughness() const { return glm::unpackHalf2x16(m_data2).y; }

void Material::setRoughness(float roughness)
{
    roughness        = glm::clamp(roughness, 0.0f, 1.0f);
    const float colB = glm::unpackHalf2x16(m_data2).x;
    m_data2    = glm::packHalf2x16(glm::vec2(colB, roughness));
}

float Material::getMetalness() const
{ return glm::unpackHalf2x16(m_data3).x; }

void Material::setMetalness(float metalness)
{
    metalness       = glm::clamp(metalness, 0.0f, 1.0f);
    const float ior = getIOR();
    m_data3         = glm::packHalf2x16(glm::vec2(metalness, ior));
}

float Material::getIOR() const
{ return glm::unpackHalf2x16(m_data3).y; }

void Material::setIOR(float ior)
{
    ior                   = glm::max(0.0f, ior);
    const float metalness = getMetalness();
    m_data3               = glm::packHalf2x16(glm::vec2(metalness, ior));
}
