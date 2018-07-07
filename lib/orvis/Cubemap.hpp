#pragma once
#include "Camera.hpp"
#include "Buffer.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "VertexArray.hpp"
#include <fstream>

using namespace gl;

/** @brief A Class to load, generate and render cubemaps.
 * @details
 */
class Cubemap : Createable<Cubemap>
{
public:
    /** @brief Creates a new shader program and gets a uniform location for invViewProj Matrix.
     * @details The Shader Program attaches skyBox.vert and skyBox.frag shader in which the cube
     * mapping is implemented.
     */
    Cubemap();

    /** @brief Loads a cubemap from a source path.
     * @param cubeMapSourcePath The Path where the cubemap faces reside.
     * Example: ..."/textures/indoor/"
     * @param extension The file extension of the cubemap. Example: ".hdr".
     * This is also the thefault value for this parameter.
     * @param posX The face of the cubemap pointing in positive x direction.
     * Example: "posx". This is also the thefault value for this parameter.
     * @param negX The face of the cubemap pointing in negative x direction.
     * Example: "negx". This is also the thefault value for this parameter.
     * @param posY The face of the cubemap pointing in positive y direction.
     * Example: "posy". This is also the thefault value for this parameter.
     * @param negY The face of the cubemap pointing in negative y direction.
     * Example: "negy". This is also the thefault value for this parameter.
     * @param posZ The face of the cubemap pointing in positive z direction.
     * Example: "posz". This is also the thefault value for this parameter.
     * @param negZ The face of the cubemap pointing in negative z direction.
     * Example: "negz". This is also the thefault value for this parameter.
     * @details In this function the strings get concatinated to load a face of the cubemap.
     * Example "/textures/indoor/posx.hdr
     */

    void generateCubeMap(const std::experimental::filesystem::path& cubeMapSourcePath,
                         const std::string& extension = ".hdr", const std::string& posX = "posx",
                         const std::string& negX = "negx", const std::string& posY = "posy",
                         const std::string& negY = "negy", const std::string& posZ = "posz",
                         const std::string& negZ = "negz");

    /** @brief Loads a cubemap from a source path.
     * @param cubeMapSourcePath The Path where the cubemap faces reside.
     * Example: ..."/textures/indoor/"
     * @param extension The file extension of the cubemap. Example: ".hdr".
     * This is also the thefault value for this parameter.
     * @param faces This is an array containing the 6 faces of the cubemap as strings.
     * Example: {"pox", "negx", "posy", "negy", "posz", "negz"}
     * @details In this function the strings get concatinated to load a face of the cubemap.
     * Example "/textures/indoor/posx.hdr
     */
    void generateCubeMap(const std::experimental::filesystem::path& path, const std::string& extension,
                         const std::array<std::string, 6>& faces);

    /** @brief Renders the cubemap into a cube.
     * Pass the uniform of the viewProj matrix to the shader.
     * @param viewMatrix. Cuts off the translation part of the view matrix internaly.
     * @param projMatrix Target projection matrix.
     */
    void render(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);

    const Texture& getTexture() const;

private:
    Texture     m_texture{GL_TEXTURE_CUBE_MAP, GL_R11F_G11F_B10F, glm::ivec2(1, 1)};
    Program     m_shaderProgram;
    GLint       m_viewProjLoc;
    VertexArray m_vao;
};
