#pragma once

#include "glsp/glsp.hpp"

enum class BufferBinding : int
{
    modelMatrices = 50,
    cameraParameters = 51,
    lights = 52,
    materials = 53,
    boundingBoxes = 54,
    indirectDraw = 55,
    lightIndex = 56,
};

enum class TextureBinding : int
{
    skybox = 50
};

enum class VertexAttributeBinding : int
{
    vertices = 0,
    normals = 1,
    texCoords = 2
};

namespace binding
{
    inline std::vector<glsp::definition> defaultShaderDefines = {
        glsp::definition("MODELMATRICES_BINDING", static_cast<int>(BufferBinding::modelMatrices)),
        glsp::definition("CAMERA_BINDING", static_cast<int>(BufferBinding::cameraParameters)),
        glsp::definition("LIGHTS_BINDING", static_cast<int>(BufferBinding::lights)),
        glsp::definition("MATERIALS_BINDING", static_cast<int>(BufferBinding::materials)),
        glsp::definition("BOUNDING_BOXES_BINDING", static_cast<int>(BufferBinding::boundingBoxes)),
        glsp::definition("INDIRECT_DRAW_BINDING", static_cast<int>(BufferBinding::indirectDraw)),
        glsp::definition("LIGHT_INDEX_BINDING", static_cast<int>(BufferBinding::lightIndex)),

        glsp::definition("SKYBOX_BINDING", static_cast<int>(TextureBinding::skybox)),

        glsp::definition("VERTEX_LAYOUT", static_cast<int>(VertexAttributeBinding::vertices)),
        glsp::definition("NORMAL_LAYOUT", static_cast<int>(VertexAttributeBinding::normals)),
        glsp::definition("TEXCOORD_LAYOUT", static_cast<int>(VertexAttributeBinding::texCoords))
    };
}
