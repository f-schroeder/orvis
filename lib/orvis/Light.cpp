#include "Light.hpp"
#include "Bounds.hpp"
#include <imgui.h>
#include <functional>

Light::Light(const Light& other)
{
    color = other.color;
    cutOff = other.cutOff;
    position = other.position;
    pcfKernelSize = other.pcfKernelSize;
    direction = other.direction;
    m_type = other.m_type;
    m_lightSpaceMatrix = other.m_lightSpaceMatrix;
    m_shadowMapHandle = other.m_shadowMapHandle;
}

Light& Light::operator=(const Light& other)
{
    color = other.color;
    cutOff = other.cutOff;
    position = other.position;
    pcfKernelSize = other.pcfKernelSize;
    direction = other.direction;
    m_type = other.m_type;
    m_lightSpaceMatrix = other.m_lightSpaceMatrix;
    m_shadowMapHandle = other.m_shadowMapHandle;
    return *this;
}

std::shared_ptr<Light> Light::makePointLight(glm::vec3 position, glm::vec3 color)
{
    return std::shared_ptr<Light>(new Light(position, glm::vec3(0.0f), color, 0.0f, LightType::point));
}

std::shared_ptr<Light> Light::makeDirectionalLight(glm::vec3 direction, glm::vec3 color)
{
    return std::shared_ptr<Light>(new Light(glm::vec3(0.0f), direction, color, 0.0f, LightType::directional));
}

std::shared_ptr<Light> Light::makeSpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float cutOff)
{
    return std::shared_ptr<Light>(new Light(position, direction, color, cutOff, LightType::spot));
}

void Light::updateShadowMap(const Scene& scene) const
{
    m_shadowMap->render(scene, m_lightSpaceMatrix);
}

void Light::recalculateLightSpaceMatrix(const Scene& scene)
{
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    if (glm::length(glm::cross(direction, up)) < 0.01f)
    {
        up = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    const Bounds& b = scene.bounds;
    const float bboxSize = glm::length(b[1] - b[0]);

    glm::mat4 view, projection;
    if (m_type == LightType::directional)
    {
        const glm::vec3 bboxCenter = 0.5f * (b[1] + b[0]);
        // position needed for shadow map
        position = bboxCenter + 0.5f * bboxSize * glm::normalize(-direction);

        const glm::vec2 bb = glm::vec2(glm::compMin(b[0]), glm::compMax(b[1]));
        const float min = bb.x - 0.244f * abs(bb.x); //0.244 = (sqrt(3)-1)/3
        const float max = bb.y + 0.244f * abs(bb.y);

        projection = glm::ortho(min, max, min, max, 0.1f, bboxSize);
        view = glm::lookAt(position, position + direction, up);
    }
    else if (m_type == LightType::spot)
    {
        // NOTE: ACOS BECAUSE CUTOFF HAS COS BAKED IN
        projection = glm::perspectiveFov(2.0f*glm::acos(glm::clamp(cutOff/* + 0.1f*/, 0.1f, 0.9f)) , static_cast<float>(m_shadowMap->shadowFBO.getDepthTexture()->getSize().x), static_cast<float>(m_shadowMap->shadowFBO.getDepthTexture()->getSize().y), 0.1f, bboxSize);
        view = glm::lookAt(position, position + direction, up);
    }
    else if (m_type == LightType::point)
    {
        // TODO is cutoff supposed to be used here? or 90 degrees (cube)?
        projection = glm::perspective(glm::radians(90.0f), static_cast<float>(m_shadowMap->shadowFBO.getDepthTexture()->getSize().x) / static_cast<float>(m_shadowMap->shadowFBO.getDepthTexture()->getSize().y), 0.1f, bboxSize);
        view = glm::mat4(1.0f); // calculate finished matrix in shader
    }

    m_lightSpaceMatrix = projection * view;
}

Light::Light(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float cutOff, LightType type) :
    color(color), cutOff(cutOff), position(position), direction(direction), m_type(type)
{
    m_shadowMap = std::make_unique<ShadowMap>();
    m_shadowMap->shadowFBO.getDepthTexture()->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_shadowMap->shadowFBO.getDepthTexture()->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_shadowMap->shadowFBO.getDepthTexture()->set(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    m_shadowMap->shadowFBO.getDepthTexture()->set(GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    m_shadowMapHandle = m_shadowMap->shadowFBO.getDepthTexture()->handle();
}

Light::ShadowMap::ShadowMap()
{
    shadowProgram.attachNew(GL_VERTEX_SHADER, ShaderFile::load("vertex/lightTransform.vert"));
    shadowProgram.attachNew(GL_FRAGMENT_SHADER, ShaderFile::load("fragment/shadowMap.frag"));
}

void Light::ShadowMap::render(const Scene& scene, const glm::mat4& lightSpaceMatrix) const
{
    // store old viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    shadowFBO.bind();

    // set SM render settings
    glViewport(0, 0, shadowFBO.getDepthTexture()->getSize().x, shadowFBO.getDepthTexture()->getSize().y);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);

    // render SM
    //shadowProgram.use();
    scene.getCamera()->uploadToGpu(glm::mat4(1.0f), lightSpaceMatrix);
    scene.render(shadowProgram, false);

    shadowFBO.getDepthTexture()->generateMipmaps();

    // restore previous render settings
    FrameBuffer::unbind();
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glCullFace(GL_BACK);
}

bool Light::drawGuiWindow()
{
    ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Light");
    const bool changed = drawGuiContent();
    ImGui::End();
    return changed;
}

bool Light::drawGuiContent()
{
    ImGui::PushID(this);

    bool changed = false;

    std::string typeString;
    switch (m_type)
    {
    case LightType::directional:
        typeString = "Directional ";
        break;
    case LightType::point:
        typeString = "Point ";
        break;
    case LightType::spot:
        typeString = "Spot ";
        break;
    }

    if (ImGui::CollapsingHeader((typeString + std::string("Light")).c_str()))
    {
        float intensity = glm::max(glm::compMax(color), 1.0f);
        if (ImGui::ColorEdit3("Color", glm::value_ptr(color), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_PickerHueWheel) ||
            ImGui::DragFloat("Intensity", &intensity, intensity/10.0f, 1.0f, 1000000000.0f))
        {
            color = color / glm::max(glm::compMax(color), 1.0f) * intensity;
            changed = true;
        }

        if (m_type == LightType::point || m_type == LightType::spot)
            changed |= ImGui::DragFloat3("Position", glm::value_ptr(position));

        if (m_type == LightType::directional || m_type == LightType::spot)
            changed |= ImGui::SliderFloat3("Direction", glm::value_ptr(direction), -1.0f, 1.0f);

        if (m_type == LightType::spot)
            changed |= ImGui::SliderFloat("Cutoff", &cutOff, 0.1f, 1.0f);

        changed |= ImGui::SliderInt("PCF size", &pcfKernelSize, 0, 10);
    }

    ImGui::PopID();

    return changed;
}
