#include "Light.hpp"
#include "Bounds.hpp"

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

void Light::update(const std::shared_ptr<Scene>& scene)
{
    recalculateLightSpaceMatrix(scene);
    m_shadowMap->render(scene);
}

void Light::recalculateLightSpaceMatrix(const std::shared_ptr<Scene>& scene)
{
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    if (glm::length(glm::cross(direction, up)) < 0.01f)
    {
        up = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    const Bounds& b = scene->bounds;
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
        projection = glm::perspective(2.0f*glm::acos(cutOff + 0.1f), static_cast<float>(m_shadowMap->shadowTexture.getSize().x) / static_cast<float>(m_shadowMap->shadowTexture.getSize().y), 0.1f, bboxSize);
        view = glm::lookAt(position, position + direction, up);
    }
    else if (m_type == LightType::point)
    {
        // TODO is cutoff supposed to be used here? or 90 degrees (cube)?
        projection = glm::perspective(glm::radians(90.0f), static_cast<float>(m_shadowMap->shadowTexture.getSize().x) / static_cast<float>(m_shadowMap->shadowTexture.getSize().y), 0.1f, bboxSize);
        view = glm::mat4(1.0f); // calculate finished matrix in shader
    }

    m_lightSpaceMatrix = projection * view;
}

Light::Light(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float cutOff, LightType type) :
    color(color), cutOff(cutOff), position(position), direction(direction), m_type(type)
{
    m_shadowMap = std::make_unique<ShadowMap>();
    m_shadowMap->shadowTexture.set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_shadowMap->shadowTexture.set(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    m_shadowMap->shadowTexture.set(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    m_shadowMap->shadowTexture.set(GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    m_shadowMapHandle = m_shadowMap->shadowTexture.handle();
}

Light::ShadowMap::ShadowMap()
{
    shadowProgram.attachNew(GL_VERTEX_SHADER, ShaderFile::load("vertex/shadowMap.vert"));
    shadowProgram.attachNew(GL_FRAGMENT_SHADER, ShaderFile::load("fragment/shadowMap.frag"));
}

void Light::ShadowMap::render(const std::shared_ptr<Scene>& scene) const
{
    // store old viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // set SM render settings
    glViewport(0, 0, shadowTexture.getSize().x, shadowTexture.getSize().y);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);

    // render SM
    shadowProgram.use();
    shadowFBO.bind();
    //scene->render();

    // restore previous render settings
    FrameBuffer::unbind();
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glCullFace(GL_BACK);
}
