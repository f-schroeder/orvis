#pragma once

#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include "Buffer.hpp"

enum class Direction
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

enum class SpeedModifier
{
    NORMAL,
    SLOW,
    FAST
};

// default camera values
constexpr float SPEED      = 10.0f;
constexpr float SENSITIVTY = 0.05f;

struct GpuCamera
{
    glm::vec3 position = {0.f, 0.f, 0.f};
    float gamma = 2.2f;
    glm::vec3 direction = {0.f, 0.f, 0.f};
    float exposure = 1.0f;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 invVP;
};

/**
 * @brief first person camera
 * @details camera movements:
 *		WASD:		move left/right/forward/backward
 *		MOUSE:		hold right mouse button to turn camera
 *		C:			toggle turning camera as alternative to right mouse button
 *		Left Shift:	speed up
 *		R/F:		move up/down
 *		T/G:		increase/decrease camera movement speed
 *		SPACE:		reset camera
 */
class Camera
{
public:
    /**
     * @brief creates camera object and sets starting parameters
     * @param projection The projection matrix of the camera
     * @param position Starting position
     * @param target Location to which the camera is pointed at
     * @param up Starting up vector
     */
    Camera(glm::mat4 projection, glm::vec3 position = glm::vec3(0.0f), glm::vec3 target = {0, 0, -1},
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));

    /**
     * @brief Handles input and updates camera values
     * @param window GLFWwindow to get inputs from
     */
    void update(GLFWwindow* window);

    /**
    * @brief Puts camera data into a GpuCamera struct and uploads it to the GPU.
    * Also binds the buffer to binding::BufferBinding::cameraParameters.
    * Only does so if the camera parameters changed since the last update.
    */
    void uploadToGpu();

    /**
    * @brief Puts camera data into a GpuCamera struct and uploads it to the GPU.
    * Only takes values from the provided matrices instead of using class-members.
    * Also binds the buffer to binding::BufferBinding::cameraParameters
    * @param view The view matrix
    * @param proj The projection matrix
    */
    void uploadToGpu(const glm::mat4& view, const glm::mat4& proj);

    /**
     * @brief Resets camera to starting position and orientation
     */
    void reset();

    /** @return The view matrix */
    glm::mat4 view() const;

    // camera attributes
    glm::vec3 position; //!< The camera position.
    glm::quat rotation; //!< The camera rotation quaternion.
    float gamma; //!< The gamma value for gamma correction.
    float exposure; //!< The exposure for tone mapping.

    /** @return The local forward vector ({0, 0, -1} rotated). */
    glm::vec3 forward() const;
    /** @return The local backward vector ({0, 0, 1} rotated). */
    glm::vec3 backward() const;
    /** @return The local up vector ({0, 1, 0} rotated). */
    glm::vec3 up() const;
    /** @return The local down vector ({0, -1, 0} rotated). */
    glm::vec3 down() const;
    /** @return The local left vector ({-1, 0, 0} rotated). */
    glm::vec3 left() const;
    /** @return The local right vector ({1, 0, 0} rotated). */
    glm::vec3 right() const;

    /** @brief Sets the projection matrix. */
    void setProjection(glm::mat4 projection);

    /** @return The view matrix. */
    glm::mat4 projection() const;

    /** @brief Sets the movement speed of the camera. */
    void setSpeed(float speed);

    /** 
    * @brief Draws a ImGui-window containing the camera parameters.  
    * @return true if the parameters were changed.
    */
    bool drawGuiWindow();

    /**
    * @brief Draws the ImGui-content containing the camera parameters.
    * @return true if the parameters were changed.
    */
    bool drawGuiContent();

private:
    void processKeyboard(Direction direction, double deltaTime, SpeedModifier speedModifier);
    void processMouseMovement(double xoffset, double yoffset);

    // starting paramters for reset
    glm::vec3 m_startPosition;
    glm::quat m_startRotation;

    // camera options
    float m_speed;
    float m_sensitivity;

    double m_lastTime;
    double m_lastX        = 0.0;
    double m_lastY        = 0.0;
    bool   m_cameraActive = false;
    bool   m_mousePressed = false;
    bool   m_cPressed     = false;

    glm::mat4 m_projection;

    Buffer<GpuCamera> m_cameraBuffer;

    size_t m_camHash;
};
