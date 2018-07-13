#include <glbinding/gl/gl.h>
#include "orvis/Cubemap.hpp"
#include "orvis/Scene.hpp"
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

    std::shared_ptr<Camera> cam = std::make_shared<Camera>(glm::perspectiveFov(glm::radians(60.f), static_cast<float>(width), static_cast<float>(height), 0.01f, 1000.f));

    Cubemap skybox;
    skybox.generateCubemap(util::resourcesPath / "textures" / "indoor");

    Program shaderProg;
    shaderProg.attachNew(GL_VERTEX_SHADER, ShaderFile::load("vertex/multiDraw.vert"));
    shaderProg.attachNew(GL_FRAGMENT_SHADER, ShaderFile::load("fragment/basicRendering.frag"));

    Scene scene("sponza/sponza.obj");
    scene.setCamera(cam);

    Timer timer;

    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);

    while(float deltatime = window.update() > 0.0f)
    {
        timer.start();

        // --- RENDERING ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->update(window);

        scene.render(shaderProg);

        //skybox.renderAsSkybox(cam);
    
        timer.stop();
        timer.drawGuiWindow(window);
    }
    return 0;
}
