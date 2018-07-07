#include "Window.hpp"
#include "Util.hpp"
#include <imgui.h>
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"
#include "Shader.hpp"

Window::Window(int width, int height, const std::string& title, const Hints& hints)
{
    glfwInit();

    setHints(hints);

    if constexpr (util::debugmode)
        setHints({ { GLFW_OPENGL_DEBUG_CONTEXT, 1 } });
    else
    {
        setHints({ { GLFW_OPENGL_DEBUG_CONTEXT, 0 }, { GLFW_CONTEXT_NO_ERROR, 1 } });
    }

    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(0);

    // init opengl
    util::initGL();

    // print OpenGL info
    util::printOpenGLInfo();

    // set up imgui
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init();
    ImGui::StyleColorsDark();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

Window::~Window() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

Window::operator GLFWwindow*() const { return m_window; }

GLFWwindow* Window::getGlfwWindowPtr() const { return m_window; }

float Window::update()
{
    int sx, sy;
    glfwGetWindowSize(m_window, &sx, &sy);
    while (sx * sy == 0)
        glfwPollEvents();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const float deltaTime = util::getTime() - m_lastTime;
    m_lastTime = util::getTime();

    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        close();

    if (glfwGetKey(m_window, GLFW_KEY_F5) == GLFW_PRESS)
        Program::reloadAll();

    glfwSwapBuffers(m_window);
    glfwPollEvents();

    return glfwWindowShouldClose(m_window) ? -1.0f : deltaTime;
}

void Window::close() const { glfwSetWindowShouldClose(m_window, true); }

void Window::setHints(const Hints& hints)
{
    for (const auto& hint : hints)
        glfwWindowHint(hint.first, hint.second);
}
