#version 430
#extension GL_ARB_bindless_texture : require

in vec2 passTexCoord;
flat in uint passDrawID;

#include "include/material.glsl"

void main()
{             
    Material mat = getMaterial(passDrawID, passTexCoord);

	if(mat.albedo.a < 0.9f)
		discard;
}  