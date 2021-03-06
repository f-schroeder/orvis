#include "Util.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <ctime>
#include <iostream>
#include <sstream>
#include <future>
#include <glbinding/Binding.h>

// Execute on dedicated graphics card if possible
#if defined(WIN32) || defined(_WIN32)
extern "C" {
    __declspec(dllexport) int NvOptimusEnablement = 0x00000001;
}
extern "C" {
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
}
#else
extern "C" {
    int NvOptimusEnablement = 1;
}
extern "C" {
    int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace util
{
    std::string convertGLubyteToString(const GLubyte* content)
    {
        return std::string(reinterpret_cast<const char*>(content));
    }

#pragma warning( push )
#pragma warning( disable : 4146) // intended use of negation operation on unsigned variable
    void setBit(GLuint& bitset, unsigned int index, bool value)
    {
        const GLuint newbit = !!value; // booleanize to force 0 or 1
        bitset ^= (-newbit ^ bitset) & (1U << index);
    }
#pragma warning( pop ) 

    bool getBit(GLuint bitset, unsigned int index)
    {
        return (bitset >> index) & 1U;
    }

    void printOpenGLInfo()
    {
        std::cout << "Renderer: " << convertGLubyteToString(glGetString(GL_RENDERER)) << std::endl;
        std::cout << "Vendor: " << convertGLubyteToString(glGetString(GL_VENDOR)) << std::endl;
        std::cout << "Version: " << convertGLubyteToString(glGetString(GL_VERSION)) << std::endl;
        std::cout << "Shading Language Version: " << convertGLubyteToString(glGetString(GL_SHADING_LANGUAGE_VERSION)) << std::endl;
    }

    float getTime()
    {
        return static_cast<float>(glfwGetTime());
    }

    void initGL()
    {
        // init glbinding
        glbinding::Binding::initialize();
    }

    std::vector<std::string> getGLExtenstions()
    {
        GLint nExtensions;
        glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
        std::vector<std::string> extenstions;
        extenstions.reserve(nExtensions);
        for (int i = 0; i < nExtensions; i++)
        {
            extenstions.push_back(std::string(convertGLubyteToString(glGetStringi(GL_EXTENSIONS, i))));
        }
        return extenstions;
    }

    void getGlError(int line, const std::string& function)
    {
        if constexpr(debugmode)
        {
            if (glfwGetCurrentContext() != nullptr)
            {
                GLenum err;
                while ((err = glGetError()) != GL_NO_ERROR)
                {
                    std::cout << "OpenGL Error: " << err << std::endl;
                    std::cout << "Last error check in function " << function << " at line " << line << std::endl;
                }
            }
        }
    }

    void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                const GLchar* message, const void* userParam)
    {
        // ignore some warnings
        if (id == 131204 /*unbound texture*/ || id == 131169 /*render buffer allocation*/ || id == 131186 /*copy from video to host*/)
            return;

        std::cout << "OpenGL debug callback called!" << '\n';
        std::cout << "Source: ";
        switch (source)
        {
        case GL_DEBUG_SOURCE_API:
            std::cout << "API Call";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            std::cout << "Window system";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            std::cout << "Third party application";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            std::cout << "This application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            std::cout << "Some other source";
            break;
        default:
            std::cout << "Invaliid source";
        }
        std::cout << '\n';
        std::cout << "message: " << message << '\n';
        std::cout << "type: ";
        // converting GLenums is tedious :(
        switch (type)
        {
        case GL_DEBUG_TYPE_ERROR:
            std::cout << "ERROR";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            std::cout << "DEPRECATED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            std::cout << "UNDEFINED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            std::cout << "PORTABILITY";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            std::cout << "PERFORMANCE";
            break;
        case GL_DEBUG_TYPE_MARKER:
            std::cout << "Annotation (MARKER)";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            std::cout << "Debug push group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            std::cout << "Debug pop group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            std::cout << "OTHER";
            break;
        default:
            std::cout << "Invalid debug type";
        }
        std::cout << '\n';
        std::cout << "id: " << id << '\n';
        std::cout << "severity: ";
        switch (severity)
        {
        case GL_DEBUG_SEVERITY_LOW:
            std::cout << "LOW";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            std::cout << "MEDIUM";
            break;
        case GL_DEBUG_SEVERITY_HIGH:
            std::cout << "HIGH";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            std::cout << "NOTIFICATION";
            break;
        default:
            std::cout << "Invalid severity";
        }
        std::cout << '\n';
        std::cout << std::endl;
    }

    void enableDebugCallback()
    {
        if constexpr(debugmode)
        {
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(debugCallback, nullptr);

            // disable notifications and memory info
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_TRUE);

            // enable more severe errors
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
        }
    }

    void savePNG(const std::string& name, std::vector<unsigned char>& image, int width, int height)
    {
        std::stringstream path;
        path << (util::resourcesPath) << "../../" << name << "_" << time(nullptr) << ".png";

        stbi_flip_vertically_on_write(true);
        const auto err = stbi_write_png(path.str().c_str(), width, height, 4, image.data(), 4 * width);
        if (err == 0)
            throw std::runtime_error("error writing image");
    }

    void saveFBOtoFile(const std::string& name, GLFWwindow* window)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        int width;
        int height;
        glfwGetFramebufferSize(window, &width, &height);
        std::vector<unsigned char> image;
        image.resize(width * height * 4);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image.data());

        //Encode the image
        try
        {
            auto future = std::async(std::launch::async, [&]() { savePNG(name, image, width, height); });
        }
        catch (std::runtime_error& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }
}
