#include "Camera.hpp"
#include <imgui.h>

Camera::Camera(glm::mat4 projection, glm::vec3 position, glm::vec3 target, glm::vec3 up)
    : position(position)
    , rotation(glm::quat_cast(glm::lookAt(position, target, up)))
    , m_startPosition(position)
    , m_startRotation(rotation)
    , m_speed(SPEED)
    , m_sensitivity(SENSITIVTY)
    , m_lastTime(glfwGetTime())
    , m_projection(projection)
    , m_cameraBuffer({ glm::vec4(position, 1.0f), glm::vec4(forward(), 0.0f), view(), projection, glm::inverse(projection * view()) }, GL_DYNAMIC_STORAGE_BIT)
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
        const bool speedMod = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
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
            updateSpeed(m_speed + 0.01f);
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
            updateSpeed(m_speed - 0.01f);
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
}

void Camera::uploadToGpu()
{
    const glm::mat4 v = view();
    const glm::mat4 p = projection();
    const glm::mat4 invVP = glm::inverse(p * v);
    m_cameraBuffer.assign({ glm::vec4(position, 1.0f), glm::vec4(forward(), 0.0f), v, p, invVP });
    m_cameraBuffer.bind(GL_UNIFORM_BUFFER, BufferBinding::cameraParameters);
}

void Camera::reset()
{
    position = m_startPosition;
    rotation = m_startRotation;
}

glm::mat4 Camera::view() const { return glm::lookAt(position, position + forward(), up()); }

void Camera::processKeyboard(Direction direction, double deltaTime, bool speedModifier)
{
    float velocity = m_speed * static_cast<float>(deltaTime);
    if (speedModifier)
        velocity *= m_speed;
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

inline void Camera::updateSpeed(float speed)
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
