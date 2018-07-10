#include "Material.hpp"
#include "Util.hpp"

Material::Material(glm::vec4 color, float roughness, float metallic, float ior)
{
    setColor(color);
    setRoughness(roughness);
    setMetallic(metallic);
    setIOR(ior);
}

glm::vec4 Material::getColor() const
{
    if (m_isTextureBitset[MAT_COLOR_BIT])
    {
        std::cout << "WARNING: Attempting to get material color although it is a texture\n";
        return glm::vec4(-1.0f);
    }
    else
    {
        return util::unpackHalf4x16(m_albedo);
    }
}

void Material::setColor(std::variant<glm::vec4, std::shared_ptr<Texture>> color)
{
    if (std::holds_alternative<glm::vec4>(color))
    {
        m_albedo = util::packHalf4x16(glm::max(glm::vec4(0.0f), std::get<glm::vec4>(color)));
        m_isTextureBitset[MAT_COLOR_BIT] = false;
    }
    else
    {
        m_albedo = util::uint64ToUvec2(std::get<std::shared_ptr<Texture>>(color)->handle());
        m_isTextureBitset[MAT_COLOR_BIT] = true;
    }
}

void Material::setNormalMap(const std::shared_ptr<Texture>& normalMap)
{
    m_normal = normalMap->handle();
    m_isTextureBitset[MAT_NORMAL_BIT] = true;
}

void Material::setAoMap(const std::shared_ptr<Texture>& aoMap)
{
    m_ao = aoMap->handle();
    m_isTextureBitset[MAT_AO_BIT] = true;
}

float Material::getRoughness() const
{
    if (m_isTextureBitset[MAT_ROUGHNESS_BIT])
    {
        std::cout << "WARNING: Attempting to get material roughness although it is a texture\n";
        return -1.0f;
    }
    else
    {
        return glm::uintBitsToFloat(m_roughness.x);
    }
}

void Material::setRoughness(std::variant<float, std::shared_ptr<Texture>> roughness)
{    
    if (std::holds_alternative<float>(roughness))
    {
        m_roughness.x = glm::floatBitsToUint(glm::clamp(std::get<float>(roughness), 0.0f, 1.0f));
        m_isTextureBitset[MAT_ROUGHNESS_BIT] = false;
    }
    else
    {
        m_roughness = util::uint64ToUvec2(std::get<std::shared_ptr<Texture>>(roughness)->handle());
        m_isTextureBitset[MAT_ROUGHNESS_BIT] = true;
    }
}

float Material::getMetallic() const
{
    if (m_isTextureBitset[MAT_METALLIC_BIT])
    {
        std::cout << "WARNING: Attempting to get material metallic although it is a texture\n";
        return -1.0f;
    }
    else
    {
        return glm::uintBitsToFloat(m_metallic.x);
    }
}

void Material::setMetallic(std::variant<float, std::shared_ptr<Texture>> metallic)
{
    if (std::holds_alternative<float>(metallic))
    {
        m_metallic.x = glm::floatBitsToUint(glm::clamp(std::get<float>(metallic), 0.0f, 1.0f));
        m_isTextureBitset[MAT_METALLIC_BIT] = false;
    }
    else
    {
        m_metallic = util::uint64ToUvec2(std::get<std::shared_ptr<Texture>>(metallic)->handle());
        m_isTextureBitset[MAT_METALLIC_BIT] = true;
    }
}

float Material::getIOR() const
{
    return m_ior;
}

void Material::setIOR(float ior)
{
    m_ior = glm::max(0.0f, ior);
}
