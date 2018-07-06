#pragma once
#include <GLFW/glfw3.h>
#include <unordered_map>

/**
 * @brief A light wrapper for GLFW windows.
 */
class Window
{
public:
    /** Maps glfw window hints to their values. */
    using Hints = std::unordered_map<int, int>;

    /**
     * @brief Creates a window. Applies all provided hints the same way as glfwWindowHint(int, int) does.
     * @param width The initial window width.
     * @param height The initial window height.
     * @param title The window title.
     * @param hints Additional GLFW window hints.
     */
    Window(int width = 1600, int height = 900, const std::string& title = "OpenGL Window", const Hints& hints = {});

    ~Window();

    operator GLFWwindow*() const;
    GLFWwindow* getGlfwWindowPtr() const;

    /**
     * @brief Polls GLFW events and swaps default framebuffers. If the window size is such that the
     * surface is invisible (i.e. one dimension is zero), update() blocks, still calling
     * glfwPollEvents(), until the window is resized back to a valid size.
     * @return positive frametime if the window should be kept open, negative if there is a hint to close the window.
     */
    float update();

    /**
     * @brief Hints a closing action for the window. Should be closed after update() returns false
     * afterwards.
     */
    void close() const;

private:
    static void setHints(const Hints& hints);

    GLFWwindow* m_window = nullptr;

    float m_lastTime = 0.0f;
};
