#pragma once

#include <vector>
#include <thread>
#include <experimental/filesystem>

#include <glbinding/gl/gl.h>
#include <glm/detail/type_vec4.hpp>
using namespace gl;
#include <GLFW/glfw3.h>

namespace util
{
    /**
     * @brief converts a GLubyte* 'char array' to a std::string
     * @param content input GLubyte* 'char array'
     * @return std::string with same text as input
     */
    std::string convertGLubyteToString(const GLubyte* content);

    /**
     * @brief Calls glm::unpackHalf2x16() on both components of v and renturns them as vec4
     */
    inline glm::vec4 unpackHalf4x16(glm::uvec2 v);

    /**
    * @brief Calls glm::packHalf2x16() on both components of v and renturns them as uvec2
    */
    inline glm::uvec2 packHalf4x16(glm::vec4 v);

    /**
    * @brief Splits a GLuint64 into a uvec2
    */
    inline glm::uvec2 uint64ToUvec2(GLuint64 v);

    /**
    * @brief Merges a uvec2 into a GLuint64
    */
    inline GLuint64 uvec2ToUitn64(glm::uvec2 v);

    /**
     * @brief prints the OpenGL driver/vendor info to the console
     */
    void printOpenGLInfo();

    /**
    * @brief Gets the system time using glfwGetTime() as float
    */
    float getTime();

    /**
     * @brief inits the graphics API
     */
    void initGL();

    /**
     * @brief queries all available OpenGL extensions
     * @return vector of extensions as strings
     */
    std::vector<std::string> getGLExtenstions();

    /**
     * @brief checks the OpenGL error stack (old way of getting errors)
     * @param line use __LINE__
     * @param function use __FUNCTION__
     */
    void getGlError(int line, const std::string& function);

    /**
     * @brief saves the FBO content to a PNG file (starts a new thread)
     * @param name output filename
     * @param window glfw window
     */
    void saveFBOtoFile(const std::string& name, GLFWwindow* window);

    /**
     * @brief enabled OpenGL debug callback (new way of getting errors)
     */
    void enableDebugCallback();

    /** 
     * @brief Calls the provided function and returns the number of milliseconds 
     * that it takes to call that function.
     * @param f function to call
     */
    template <class Function>
    double timeCall(Function&& f)
    {
        const auto begin = glfwGetTime();
        f();
        glFinish();
        return (glfwGetTime() - begin) * 1000;
    }

    /**
    * @brief Container for overloaded functions/lambdas
    */
    template<class... Fs>
    struct Overload : Fs...
    {
        Overload(Fs&&... fs)
            : Fs(std::move(fs))...
        {}
    };

    /**
    * @brief Makes an overloaded function/lambda object
    * @param fs The functions to be overloaded
    */
    auto const makeOverload = [](auto... fs)
    {
        return Overload<decltype(fs)...>{std::move(fs)...};
    };

    static const std::experimental::filesystem::path shadersPath = std::experimental::filesystem::current_path().parent_path().parent_path().append("shd");
    static const std::experimental::filesystem::path resourcesPath = std::experimental::filesystem::current_path().parent_path().parent_path().append("res");

#ifdef _DEBUG
    static constexpr bool debugmode = true;
#else
	static constexpr bool debugmode = false;
#endif

}
