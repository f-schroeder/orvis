#include "Material.hpp"
#include "Util.hpp"
#include <imgui.h>

Material::Material(glm::vec4 color, float roughness, float metallic, float ior)
{
    setColor(color);
    setRoughness(roughness);
    setMetallic(metallic);
    setIOR(ior);
}

glm::vec4 Material::getColor() const
{
    if (util::getBit(m_isTextureBitset, MAT_COLOR_BIT))
    {
        std::cout << "WARNING: Attempting to get material color although it is a texture\n";
        return glm::vec4(-1.0f);
    }
    else
    {
        return glm::unpackHalf4x16(glm::packUint2x32(m_albedo));
    }
}

void Material::setColor(std::variant<glm::vec4, std::shared_ptr<Texture>> color)
{
    if (std::holds_alternative<glm::vec4>(color))
    {
        //m_albedo = util::packHalf4x16(glm::max(glm::vec4(0.0f), std::get<glm::vec4>(color)));
        m_albedo = glm::unpackUint2x32(glm::packHalf4x16(glm::max(glm::vec4(0.0f), std::get<glm::vec4>(color))));
        util::setBit(m_isTextureBitset, MAT_COLOR_BIT, false);
    }
    else
    {
        m_albedo = glm::unpackUint2x32(std::get<std::shared_ptr<Texture>>(color)->handle());
        util::setBit(m_isTextureBitset, MAT_COLOR_BIT, true);
    }
}

void Material::setNormalMap(const std::shared_ptr<Texture>& normalMap)
{
    m_normal = normalMap->handle();
    util::setBit(m_isTextureBitset, MAT_NORMAL_BIT, true);
}

void Material::setAoMap(const std::shared_ptr<Texture>& aoMap)
{
    m_ao = aoMap->handle();
    util::setBit(m_isTextureBitset, MAT_AO_BIT, true);
}

float Material::getRoughness() const
{
    if (util::getBit(m_isTextureBitset, MAT_ROUGHNESS_BIT))
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
        util::setBit(m_isTextureBitset, MAT_ROUGHNESS_BIT, false);
    }
    else
    {
        m_roughness = glm::unpackUint2x32(std::get<std::shared_ptr<Texture>>(roughness)->handle());
        util::setBit(m_isTextureBitset, MAT_ROUGHNESS_BIT, true);
    }
}

float Material::getMetallic() const
{
    if (util::getBit(m_isTextureBitset, MAT_METALLIC_BIT))
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
        util::setBit(m_isTextureBitset, MAT_METALLIC_BIT, false);
    }
    else
    {
        m_metallic = glm::unpackUint2x32(std::get<std::shared_ptr<Texture>>(metallic)->handle());
        util::setBit(m_isTextureBitset, MAT_METALLIC_BIT, true);
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

bool Material::drawGuiWindow()
{
    ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Material");
    const bool changed = drawGuiContent();
    ImGui::End();
    return changed;
}

bool Material::drawGuiContent()
{
    ImGui::PushID(this);

    bool changed = false;

    if (ImGui::CollapsingHeader(std::string("Material").c_str()))
    {
        if (!util::getBit(m_isTextureBitset, MAT_COLOR_BIT))
        {
            glm::vec4 color = getColor();
            if (ImGui::ColorEdit4("Albedo", glm::value_ptr(color), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel))
            {
                setColor(color);
                changed = true;
            }
        }

        if (!util::getBit(m_isTextureBitset, MAT_ROUGHNESS_BIT))
        {
            float r = getRoughness();
            if (ImGui::SliderFloat("Roughness", &r, 0.0f, 1.0f))
            {
                setRoughness(r);
                changed = true;
            }
        }

        if (!util::getBit(m_isTextureBitset, MAT_METALLIC_BIT))
        {
            float m = getMetallic();
            if (ImGui::SliderFloat("Metallic", &m, 0.0f, 1.0f))
            {
                setMetallic(m);
                changed = true;
            }
        }

        changed |= ImGui::DragFloat("IOR", &m_ior, 0.001f, 0.0f, 10.0f);
    }

    ImGui::PopID();

    return changed;
}
