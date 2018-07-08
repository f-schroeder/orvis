#pragma once

struct CameraData
{
	vec4 position;
	vec4 direction;
	mat4 view;
	mat4 projection;
	mat4 invVP;
};

layout(std140, binding = CAMERA_BINDING) uniform CameraBuffer
{
    CameraData camera;
};
