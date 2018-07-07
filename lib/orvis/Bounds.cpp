#include "Bounds.hpp"

Bounds& Bounds::unite(const Bounds& other)
{
    min = glm::min(min, other.min);
    max = glm::max(max, other.max);
    return *this;
}

Bounds& Bounds::unite(const glm::vec3& point)
{
    min = glm::min(min, point);
    max = glm::max(max, point);
    return *this;
}

Bounds& Bounds::clip(const Bounds& other)
{
    min = glm::clamp(min, other.min, other.max);
    max = glm::clamp(max, other.min, other.max);
    return *this;
}

Bounds& Bounds::transform(const glm::mat4& transform)
{
    Bounds temp = *this;
    min         = glm::vec3(std::numeric_limits<float>::max());
    max         = glm::vec3(std::numeric_limits<float>::lowest());
    for(unsigned corner = 0; corner < 8; ++corner)
    {
        const unsigned factor_x = (corner & 0b001);
        const unsigned factor_y = (corner & 0b010) >> 1;
        const unsigned factor_z = (corner & 0b100) >> 2;

        unite(transform * glm::vec4(temp[factor_x].x, temp[factor_y].y, temp[factor_z].z, 1));
    }

    return *this;
}

bool Bounds::contains(const Bounds& other) const
{
    return glm::all(glm::greaterThanEqual(other.min, min)) &&
           glm::all(glm::lessThanEqual(other.max, max));
}

bool Bounds::contains(const glm::vec3& point) const
{
    return glm::all(glm::greaterThanEqual(point, min)) && glm::all(glm::lessThanEqual(point, max));
}

glm::vec3 Bounds::size() const { return max - min; }

glm::vec3 Bounds::center() const { return 0.5f * (min + max); }

const glm::vec3& Bounds::operator[](int index) const
{
    assert(index == 0 || index == 1 && "Index out of bounds.");
    return index == 0 ? min : max;
}

glm::vec3& Bounds::operator[](int index)
{
    assert(index == 0 || index == 1 && "Index out of bounds.");
    return index == 0 ? min : max;
}

bool Bounds::operator==(const Bounds& other) const { return min == other.min && max == other.max; }

bool Bounds::operator!=(const Bounds& other) const { return !operator==(other); }
