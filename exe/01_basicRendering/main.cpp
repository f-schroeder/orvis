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
    Window window(width, height);

    util::enableDebugCallback();

    auto cam = std::make_shared<Camera>(glm::infinitePerspective(glm::radians(60.f), static_cast<float>(width) / static_cast<float>(height), 0.1f));

    Cubemap skybox;
    skybox.generateCubemap(util::resourcesPath / "textures/rustig/hdr");

    Program shaderProg;
    shaderProg.attachNew(GL_VERTEX_SHADER, ShaderFile::load("vertex/multiDraw.vert"));
    shaderProg.attachNew(GL_FRAGMENT_SHADER, ShaderFile::load("fragment/basicRendering.frag"));

    Scene scene("sponza/sponza.obj");
    scene.reorderMeshes();
    scene.setCamera(cam);

    //auto l1 = Light::makePointLight({ 0.0f, 100.0f, 0.0f }, glm::vec3(100000.0f));
    auto l2 = Light::makeDirectionalLight();
    auto l3 = Light::makeSpotLight({ 0.0f, 100.0f, 0.0f }, { -1,-1,-1 }, glm::vec3(100000.0f));
    //scene.addLight(l1);
    
    scene.addLight(l3);
    scene.addLight(l2);

    Timer timer;

    while (float deltatime = window.update() > 0.0f)
    {
        timer.start();

        // --- RENDERING ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->update(window);
        cam->drawGuiWindow();

        skybox.renderAsSkybox(cam);

        scene.render(shaderProg);

        if (/*l1->drawGuiWindow() ||*/ l2->drawGuiWindow() || l3->drawGuiWindow())
        {
            scene.updateLightBuffer();
            scene.updateShadowMaps();
        }

        //if (scene.getMeshes()[0]->material.drawGuiWindow())
        //    scene.updateMaterialBuffer();

        timer.stop();
        timer.drawGuiWindow(window);
    }
    return 0;
}
