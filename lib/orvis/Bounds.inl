#pragma once

constexpr Bounds::Bounds()
        : min(std::numeric_limits<float>::max())
        , max(std::numeric_limits<float>::lowest())
{
}

constexpr Bounds::Bounds(const glm::vec3& min, const glm::vec3& max)
        : min(min)
        , max(max)
{
}
