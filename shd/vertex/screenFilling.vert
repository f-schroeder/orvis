#version 430 core
layout(location = 0) out vec3 rayDirection;

#include "include/camera.glsl"

void main()
{
    gl_Position = vec4(mix(-1, 3, gl_VertexID & 0x1), mix(-1, 3, (gl_VertexID >> 1) & 0x1), 0.f, 1.f);
    vec4 position = vec4(gl_Position.xy, 0.0f, 1.0f);
	position = camera.invVP * position;
	position.xyz /= position.w;
	rayDirection = position.xyz;
}
