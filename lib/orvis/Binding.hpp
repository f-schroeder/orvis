#pragma once

#include "glsp/glsp.hpp"

namespace binding
{
    enum class BufferBinding : int
    {
        modelMatrices = 50,
        cameraParameters = 51,
        lights = 52,
        materials = 53,
        materialIndices = 54
    };

    enum class TextureBinding : int
    {
        skybox = 50
    };

    enum class VertexAttributeLocation : int
    {
        vertices = 0,
        normals = 1,
        texCoords = 2
    };

    inline std::vector<glsp::definition> defaultShaderDefines = {
        glsp::definition("MODELMATRICES_BINDING", static_cast<int>(BufferBinding::modelMatrices)),
        glsp::definition("CAMERA_BINDING", static_cast<int>(BufferBinding::cameraParameters)),
        glsp::definition("LIGHTS_BINDING", static_cast<int>(BufferBinding::lights)),
        glsp::definition("MATERIALS_BINDING", static_cast<int>(BufferBinding::materials)),
        glsp::definition("MATERIAL_INDICES_BINDING", static_cast<int>(BufferBinding::materialIndices)),

        glsp::definition("SKYBOX_BINDING", static_cast<int>(TextureBinding::skybox)),

        glsp::definition("VERTEX_LAYOUT", static_cast<int>(VertexAttributeLocation::vertices)),
        glsp::definition("NORMAL_LAYOUT", static_cast<int>(VertexAttributeLocation::normals)),
        glsp::definition("TEXCOORD_LAYOUT", static_cast<int>(VertexAttributeLocation::texCoords))
    };
}
