#include <glbinding/gl/gl.h>
using namespace gl;

#include "orvis/Window.hpp"
#include "orvis/Util.hpp"
#include "orvis/Timer.hpp"

constexpr int width = 1600;
constexpr int height = 900;

int main()
{
    // init glfw, open window, manage context
    Window window (width, height);

    util::enableDebugCallback();

    Timer timer;

    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);

    while(float deltatime = window.update() > 0.0f)
    {
        timer.start();

        // --- RENDERING ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        timer.stop();
        timer.drawGuiWindow(window);
    }
    return 0;
}
