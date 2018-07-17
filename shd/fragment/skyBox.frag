#version 430

layout(location = 0) in vec3 rayDirection;
layout(location = 0) out vec4 fragColor;
layout(binding = SKYBOX_BINDING) uniform samplerCube cubeTexture;

#include "include/camera.glsl"

void main() 
{
	fragColor = texture(cubeTexture, rayDirection);

	// tone mapping
	fragColor.xyz = vec3(1.0f) - exp(-fragColor.xyz * camera.exposure);

	// gamma
    fragColor.xyz = pow(fragColor.xyz, vec3(1.0f/camera.gamma));
}