#version 460
layout (location = VERTEX_LAYOUT) in vec4 vertexPosition;
layout (location = TEXCOORD_LAYOUT) in vec2 vertexTexCoord;

#include "include/light.glsl"

layout (std430, binding = MODELMATRICES_BINDING) readonly buffer ModelMatrixBuffer
{
    mat4 modelMatrices[];
};

layout (std140, binding = LIGHT_INDEX_BINDING) uniform LightIndexBuffer
{
    int lightIndex;
};

out vec2 passTexCoord;
flat out uint passDrawID;

void main()
{
    gl_Position = lights[lightIndex].lightSpaceMatrix * modelMatrices[gl_DrawID] * vertexPosition;
	passDrawID = gl_DrawID;
	passTexCoord = vertexTexCoord;
}