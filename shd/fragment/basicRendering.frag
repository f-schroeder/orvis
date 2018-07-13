#version 460
#extension GL_ARB_bindless_texture : require
layout(early_fragment_tests) in;

layout(location = 0) in vec3 worldPos;
layout(location = 1) in vec3 viewPos;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCoord;
layout(location = 4) flat in uint drawID;

#include "include/material.glsl"

out vec4 fragColor;

void main()
{
	Material mat = getMaterial(drawID, texCoord);
	fragColor = mat.albedo;
}