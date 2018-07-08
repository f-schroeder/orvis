#version 430

layout(location = 0) in vec3 rayDirection;
layout(location = 0) out vec4 fragColor;
layout(binding = SKYBOX_BINDING) uniform samplerCube cubeTexture;

void main() 
{
	fragColor = texture(cubeTexture, rayDirection);
}