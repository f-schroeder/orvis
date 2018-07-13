#version 460 

layout (location = VERTEX_LAYOUT) in vec4 vertexPosition;
layout (location = NORMAL_LAYOUT) in vec4 vertexNormal;
layout (location = TEXCOORD_LAYOUT) in vec2 vertexTexCoord;

#include "include/camera.glsl"

layout(location = 0) out vec3 worldPos;
layout(location = 1) out vec3 viewPos;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec2 texCoord;
layout(location = 4) flat out uint drawID;

layout (std430, binding = MODELMATRICES_BINDING) readonly buffer ModelMatrixBuffer
{
    mat4 modelMatrices[];
};

void main()
{
    mat4 modelMatrix = modelMatrices[gl_DrawID];
    drawID = gl_DrawID;

    worldPos = (modelMatrix * vertexPosition).xyz;

    viewPos = (camera.view * vec4(worldPos, 1.0f)).xyz;

    gl_Position = camera.projection * vec4(viewPos, 1.0f);

    normal = mat3(transpose(inverse(modelMatrix))) * vertexNormal.xyz;
    texCoord = vertexTexCoord;
}