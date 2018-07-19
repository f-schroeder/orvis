#pragma once

#include <glbinding/gl/gl.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "Binding.hpp"

using namespace gl;

/** @brief A file handle for shader source files.
 * @details When reloading, it uses the last file write time of the
 * file to determine whether it needs to be reloaded. All files are loaded once and are then stored
 * statically and uniquely in a map keyed by their path string. This process is thread-protected by
 * a static mutex.
 *
 * Because of this behavior, the constructor is kept private and you have to use
 * ShaderFile::load(const fs::path&) to load a file.
 */
class ShaderFile
{
public:
    /** @brief Loads a shader source file from the file system if it is not already loaded yet. If
     * it has been changed since the last loading, it is reloaded.
     *
     * @param path A path to the file to load. Can be relative to the binary or relative to
     * util::shadersPath. Binary-relative files are preferred before checking for existence in
     * util::shadersPath.
     * @return The loaded file or nullptr if it has not been found.
     */
    static std::shared_ptr<ShaderFile> load(const std::experimental::filesystem::path& path);

    /** @brief Reloads this file if the time stamp (last write time) has changed. */
    void reload();

    /** @return The file content string. */
    const std::string& contents() const;

private:
    explicit ShaderFile(std::experimental::filesystem::path path);
    struct FileContent
    {
        std::shared_ptr<ShaderFile> baseHandle;
    };
    static std::unordered_map<std::string, FileContent> m_contents;
    std::experimental::filesystem::path                 m_path;
    std::string                                         m_source;
    std::experimental::filesystem::file_time_type       m_timestamp;
};

/** @brief A shader object handles compilation of OpenGL shaders and can be attached as a shared_ptr
 * to a Program.
 */
class Shader
{
public:
    /** A shader can use files or raw source strings as source. */
    using ShaderSource = std::variant<std::shared_ptr<ShaderFile>, std::string>;

    /** @brief Takes in one source file or string and compiles a shader from it.
     * @param type The OpenGL shader type. Can be one of GL_VERTEX_SHADER, GL_FRAGMENT_SHADER,
     * GL_TESSELATION_CONTROL_SHADER, GL_TESSELATION_EVALUATION_SHADER, GL_GEOMETRY_SHADER or
     * GL_COMPUTE_SHADER.
     * @param source A Shader source string or file-pointer to the shader source.
     * @param definitions Defines that are added to the shader
     */
    Shader(GLenum type, const ShaderSource& source, const std::vector<glsp::definition>& definitions = binding::defaultShaderDefines);

    /** @brief Takes in multiple source files and strings and compiles a shader from it.
     * @param type The OpenGL shader type. Can be one of GL_VERTEX_SHADER, GL_FRAGMENT_SHADER,
     * GL_TESSELATION_CONTROL_SHADER, GL_TESSELATION_EVALUATION_SHADER, GL_GEOMETRY_SHADER or
     * GL_COMPUTE_SHADER.
     * @param sources Shader source string and/or file-pointers to the shader sources. They will be
     * appended to one-another in the sequence they are passed in.
     * @param definitions Defines that are added to the shader
     */
    Shader(GLenum type, const std::vector<ShaderSource>& sources, std::vector<glsp::definition> definitions = binding::defaultShaderDefines);

    /** @brief Calls glDeleteShader, if the handle is not 0. */
    ~Shader();

    /** @brief When copy-constructing shaders, the copy target will be a new shader with a different
     * ID using the same type and source strings, but the same file pointers.
     */
    Shader(const Shader& other);

    /** @brief When move-constructing shaders, the move target will be a shader having the same ID,
     * type and sources. The old old move source shader's ID will be invalidated to 0 so it does not
     * delete the OpenGL shader associated with the ID.
     */
    Shader(Shader&& other) noexcept;

    /** @brief When copy-assigning shaders, the copy target will be a new shader with the same ID as
     * it held before copying, using the same type, copies of the source strings, and the same file
     * pointers. The old copy target shader will be deleted if not already invalidated. If the old
     * ID of the the assignment target is 0, a new ID will be generated.
     */
    Shader& operator=(const Shader& other);

    /** @brief When move-assigning shaders, the move target will be a shader having the same ID,
     * type and sources. The old move source shader's ID will be invalidated to 0 so it does not
     * delete the OpenGL shader associated with the ID. The old move target shader will be deleted
     * if not already invalidated.
     */
    Shader& operator=(Shader&& other) noexcept;

    /** @brief Reloads all ShaderFile pointers assigned to this shader and recompiles it.
     * @param checkStatus If set to true, the shader compilation status will be fetched and a log is
     * printed if an error occurs.
     * @return true, if the compilation was successful or if checkStatus is set to false, false is
     * returned otherwise.
     */
    bool reload(bool checkStatus = true);

    /**@return The OpenGL shader type set in the constructor. */
    GLenum type() const;

    /**@return The OpenGL shader name (ID). */
    GLuint id() const;

private:
    GLenum                          m_type;
    GLuint                          m_handle;
    std::vector<ShaderSource>       m_sources;
    std::vector<glsp::definition>   m_definitions;
};

/** @brief A Program consists of multiple shaders.
 * @details There are several limits to attaching shaders:
 * - If a program contains a vertex shader, a fragment shader has to be provided too.
 * - If a program contains any tesselation shader (control or evaluation), the other one has to be
 * provided too.
 * - A program only contains one of any shader type.
 *
 * All programs are kept statically in a vector as long as they are instantiated and remove
 * themselves on deletion. This is needed for a global shader program reload support. Operations on
 * this static vector are thread-safe.
 */
class Program
{
public:
    Program();
    /** @brief A program will call glDeleteProgram if the ID is not 0. */
    ~Program();

    /** @brief When copy-constructing a program, all shader pointers from the other one are
     * attached. If the other program has been linked before, the copy will be linked too. A new
     * program ID will be used for the copy.
     *
     * No states (uniform values, bindings etc.) will be copied.
     */
    Program(const Program& other);

    /** @brief When move-constructing a program, all shader pointers are moved to the copy. The old
     * ID will be used in the new copy and the old one will be invalidated to 0 to prohibit
     * deletion of the then used ID.
     */
    Program(Program&& other) noexcept;

    /** @brief When copy-assigning a program, all shader pointers from the other one are attached.
     * If the other program has been linked before, the copy will be linked too. The old program ID
     * will be reused for the copy. If the old ID of the the assignment target is 0, a new ID will
     * be generated.
     *
     * No states (uniform values, bindings etc.) will be copied.
     */
    Program& operator=(const Program& other);

    /** @brief When move-assigning a program, all shader pointers are moved to the copy. The old
     * ID will be used in the new copy and the old one will be invalidated to 0 to prohibit
     * deletion of the then used ID.
     * If the move target had a valid ID before, it will be deleted.
     */
    Program& operator=(Program&& other) noexcept;

    /** @brief Creates a new shader and attaches it to the program.
     * @param type The OpenGL shader type. Can be one of GL_VERTEX_SHADER, GL_FRAGMENT_SHADER,
     * GL_TESSELATION_CONTROL_SHADER, GL_TESSELATION_EVALUATION_SHADER, GL_GEOMETRY_SHADER or
     * GL_COMPUTE_SHADER.
     * @param source A Shader source string or file-pointer to the shader source.
     * @return A newly created shader pointer.
     */
    const std::shared_ptr<Shader>& attachNew(GLenum type, const Shader::ShaderSource& source);

    /** @brief Creates a new shader and attaches it to the program.
     * @param type The OpenGL shader type. Can be one of GL_VERTEX_SHADER, GL_FRAGMENT_SHADER,
     * GL_TESSELATION_CONTROL_SHADER, GL_TESSELATION_EVALUATION_SHADER, GL_GEOMETRY_SHADER or
     * GL_COMPUTE_SHADER.
     * @param sources Shader source strings and/or file-pointers to the shader source.
     * @return A newly created shader pointer.
     */
    const std::shared_ptr<Shader>& attachNew(GLenum                                   type,
        const std::vector<Shader::ShaderSource>& sources);

    /** @brief Attaches an existing shader to the program.
     * @param shader The shader pointer. Must not be nullptr.
     * @return The emplaced shader pointer reference.
     */
    const std::shared_ptr<Shader>& attach(const std::shared_ptr<Shader>& shader);

    /** @brief Links the program. (glLinkProgram)
     * @param checkStatus If set to true, the program link status will be fetched and a log is
     * printed if an error occurs.
     * @return true, if the linkage was successful or if checkStatus is set to false, false is
     * returned otherwise.
     */
    [[maybe_unused]] bool link(bool checkStatus = true) const;

    /** @brief Sets the program active with glUseProgram. */
    void use() const;

    /** @return The OpenGL shader program name (ID). */
    GLuint id() const;

    /** @brief Reloads all attached shaders and links the program if it has been linked before.
     * @param checkStatus Passed to Program::link(bool) and Shader::reload(bool) to
     * enable error logging.
     * @return true, if the compilation and linkage was successful or if checkStatus is set to
     * false, false is returned otherwise.
     */
    bool reload(bool checkStatus = true);

    /** @brief Reloads all created programs residing in the static vector m_allPrograms.
     * @param checkStatus Passed to Program::link(bool) and Shader::reload(bool) to
     * enable error logging.
     * @return true, if the compilation and linkage of all programs and shaders was successful or if
     * checkStatus is set to false, false is returned otherwise.
     */
    static bool reloadAll(bool checkStatus = true);

private:
    static std::mutex                                   m_programMutex;
    static std::vector<Program*>                        m_allPrograms;
    GLuint                                              m_handle;
    std::unordered_map<GLenum, std::shared_ptr<Shader>> m_shaders;
    mutable bool                                        m_linked;
};
