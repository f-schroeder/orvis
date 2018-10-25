#pragma once
#include <glm/glm.hpp>

struct Vertex
{
public:
	Vertex() = default;

	Vertex(glm::vec4 position, glm::vec4 normal, glm::vec2 uv) : position(position), normal(normal), uv(uv) {};

	glm::vec4 position = glm::vec4(0.0f);
	glm::vec4 normal = glm::vec4(0.0f);
	glm::vec2 uv = glm::vec2(0.0f);

private:

};
