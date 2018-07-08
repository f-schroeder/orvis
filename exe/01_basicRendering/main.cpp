#include <glbinding/gl/gl.h>
#include "orvis/Cubemap.hpp"
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

    std::shared_ptr<Camera> cam = std::make_shared<Camera>(glm::perspectiveFov(glm::radians(70.f), static_cast<float>(width), static_cast<float>(height), 0.01f, 1000.f));

    Cubemap skybox;
    skybox.generateCubemap(util::resourcesPath / "textures" / "indoor");

    Timer timer;

    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);

    while(float deltatime = window.update() > 0.0f)
    {
        timer.start();

        // --- RENDERING ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->update(window);
        skybox.renderAsSkybox(cam);
    
        timer.stop();
        timer.drawGuiWindow(window);
    }
    return 0;
}
