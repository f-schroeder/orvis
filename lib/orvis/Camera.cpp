#include "Camera.hpp"
#include <imgui.h>
#include <functional>
#include <glm/gtx/hash.hpp>

Camera::Camera(glm::mat4 projection, glm::vec3 position, glm::vec3 target, glm::vec3 up)
    : position(position)
    , rotation(glm::quat_cast(glm::lookAt(position, target, up)))
    , gamma(2.2f)
    , exposure(1.0f)
    , m_startPosition(position)
    , m_startRotation(rotation)
    , m_speed(SPEED)
    , m_sensitivity(SENSITIVTY)
    , m_lastTime(glfwGetTime())
    , m_projection(projection)
    , m_cameraBuffer({ position, gamma, forward(), exposure, view(), projection, glm::inverse(projection * glm::mat4(glm::mat3(view()))) }, GL_DYNAMIC_STORAGE_BIT)
{
}

void Camera::update(GLFWwindow* window)
{
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        // update time
        const auto currentFrameTime = glfwGetTime();
        const auto deltaTime = currentFrameTime - m_lastTime;
        m_lastTime = currentFrameTime;

        // check keyboard input
        const SpeedModifier speedMod = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? SpeedModifier::FAST : glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? SpeedModifier::SLOW : SpeedModifier::NORMAL;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            processKeyboard(Direction::FORWARD, deltaTime, speedMod);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            processKeyboard(Direction::BACKWARD, deltaTime, speedMod);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            processKeyboard(Direction::LEFT, deltaTime, speedMod);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            processKeyboard(Direction::RIGHT, deltaTime, speedMod);
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            processKeyboard(Direction::DOWN, deltaTime, speedMod);
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            processKeyboard(Direction::UP, deltaTime, speedMod);
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
            setSpeed(m_speed + 0.01f);
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
            setSpeed(m_speed - 0.01f);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            reset();
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !m_cPressed)
        {
            m_cPressed = true;
            m_cameraActive = !m_cameraActive;
            glfwGetCursorPos(window, &m_lastX, &m_lastY);
        }
        else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
            m_cPressed = false;
        // check mouse input
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !m_mousePressed)
        {
            m_mousePressed = true;
            m_cameraActive = true;
            glfwGetCursorPos(window, &m_lastX, &m_lastY);
        }
        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && m_mousePressed)
        {
            m_mousePressed = false;
            m_cameraActive = false;
        }

        if (m_cameraActive)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            const double xoffset = xpos - m_lastX;
            const double yoffset = m_lastY - ypos;
            m_lastX = xpos;
            m_lastY = ypos;

            processMouseMovement(xoffset, yoffset);
        }
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    uploadToGpu();
}

void Camera::uploadToGpu()
{
    const size_t hash = std::hash<glm::vec3>{}(position) + std::hash<glm::quat>{}(rotation) + std::hash<float>{}(gamma) + std::hash<float>{}(exposure);

    if (m_camHash != hash)
    {
        const glm::mat4 v = view();
        const glm::mat4 p = projection();
        const glm::mat4 invVP = glm::inverse(p * glm::mat4(glm::mat3(v)));
        m_cameraBuffer.assign({ position, gamma, forward(), exposure, v, p, invVP });
        m_cameraBuffer.bind(GL_UNIFORM_BUFFER, BufferBinding::cameraParameters);

        m_camHash = hash;
    }
}

void Camera::uploadToGpu(const glm::mat4& view, const glm::mat4& proj)
{
    const glm::mat4 invVP = glm::inverse(proj * glm::mat4(glm::mat3(view)));
    m_cameraBuffer.assign({ glm::vec3(view[3]), gamma, glm::vec3(glm::inverse(view)[2]), exposure, view, proj, invVP });
    m_cameraBuffer.bind(GL_UNIFORM_BUFFER, BufferBinding::cameraParameters);
    m_camHash = 0;
}

void Camera::reset()
{
    position = m_startPosition;
    rotation = m_startRotation;
}

glm::mat4 Camera::view() const { return glm::lookAt(position, position + forward(), up()); }

void Camera::processKeyboard(Direction direction, double deltaTime, SpeedModifier speedModifier)
{
    float velocity = m_speed * static_cast<float>(deltaTime);
    if (speedModifier == SpeedModifier::FAST)
        velocity *= 10.0f;
    else if (speedModifier == SpeedModifier::SLOW)
        velocity /= 10.0f;

    switch (direction)
    {
    case Direction::FORWARD:
        position += forward() * velocity;
        break;
    case Direction::BACKWARD:
        position += backward() * velocity;
        break;
    case Direction::RIGHT:
        position += right() * velocity;
        break;
    case Direction::LEFT:
        position += left() * velocity;
        break;
    case Direction::UP:
        position += up() * velocity;
        break;
    case Direction::DOWN:
        position += down() * velocity;
        break;
    default:
        break;
    }
}

void Camera::processMouseMovement(double xoffset, double yoffset)
{
    xoffset *= this->m_sensitivity;
    yoffset *= this->m_sensitivity;

    rotation = glm::angleAxis(glm::radians(static_cast<float>(xoffset)), glm::vec3(0, -1, 0)) *
        rotation;
    rotation = rotation *
        glm::angleAxis(glm::radians(static_cast<float>(yoffset)), glm::vec3(1, 0, 0));
}

void Camera::setSpeed(float speed)
{
    if (speed > 0)
        this->m_speed = speed;
}

glm::vec3 Camera::forward() const { return rotation * glm::vec3(0, 0, -1); }
glm::vec3 Camera::backward() const { return rotation * glm::vec3(0, 0, 1); }
glm::vec3 Camera::up() const { return rotation * glm::vec3(0, 1, 0); }
glm::vec3 Camera::down() const { return rotation * glm::vec3(0, -1, 0); }
glm::vec3 Camera::left() const { return rotation * glm::vec3(-1, 0, 0); }
glm::vec3 Camera::right() const { return rotation * glm::vec3(1, 0, 0); }

void Camera::setProjection(glm::mat4 projection)
{
    m_projection = projection;
}

glm::mat4 Camera::projection() const
{
    return m_projection;
}

bool Camera::drawGuiWindow()
{
    ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Camera");
    const bool changed = drawGuiContent();
    ImGui::End();
    return changed;
}

bool Camera::drawGuiContent()
{
    ImGui::PushID(this);

    bool changed = false;

    changed |= ImGui::DragFloat("Gamma", &gamma, 0.1f, 0.0f, 10.0f);
    changed |= ImGui::DragFloat("Exposure", &exposure, 0.1f, 0.0f, 10.0f);

    if (changed)
    {
        gamma = glm::max(gamma, 0.0f);
        exposure = glm::max(exposure, 0.0f);
        uploadToGpu();
    }

    ImGui::PopID();

    return changed;
}
