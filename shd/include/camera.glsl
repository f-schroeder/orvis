#pragma once

struct CameraData
{
	vec3 position;
	float gamma;
	vec3 direction;
	float exposure;
	mat4 view;
	mat4 projection;
	mat4 invVP;
};

layout(std140, binding = CAMERA_BINDING) uniform CameraBuffer
{
    CameraData camera;
};
