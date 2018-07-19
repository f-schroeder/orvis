#pragma once

#include <glm/glm.hpp>

/** @brief The bounding box contains a min position and a max position.
 * @details min and max are glm::vec3 types, but are still aligned to 16-byte boundaries to
 * accomplish easy GPU-buffer support. Can be put into a buffer without modifications.
 */
struct Bounds
{
    /** @brief Sets min to std::numeric_limits<float>::max() and max to
     * std::numeric_limits<float>::min(), such that any operation on this bounds will resize it to a
     * proper size.
     */
    constexpr Bounds();

    /** @brief Constructs a bounding box by its minimum and maximum positions. */
    constexpr Bounds(const glm::vec3& min, const glm::vec3& max);

    /** @brief Modifies the bounding box such that it can contain "other".
     * @param other the bounding box to be contained.
     * @return Itself.
     */
    Bounds& unite(const Bounds& other);

    /** @brief Modifies the bounding box such that it can contain "point".
     * @param point the position vector to be contained.
     * @return Itself.
     */
    Bounds& unite(const glm::vec3& point);

    /** @brief Modifies the bounding box such that it is resized to be the intersection of itself
     * with "other".
     * @param other The clip bounds.
     * @return Itself.
     */
    Bounds& clip(const Bounds& other);

    /** @brief First transforms the bounding box with a matrix to an object-oriented-bounding-box
     * and then fits it back into an axis-aligned-bounding-box again.
     * @param transform The world transformation matrix.
     * @return Itself.
     */
    Bounds& transform(const glm::mat4& transform);

    /** @brief Check whether a this bounding box contains another one.
     * @param other The bounding box to check for.
     * @return true if the bounding box fully encloses "other".
     */
    bool contains(const Bounds& other) const;

    /** @brief Check whether a this bounding box contains a position vector.
     * @param point The position to check for.
     * @return true if the bounding box fully encloses "point".
     */
    bool contains(const glm::vec3& point) const;

    /** @return the bounding box dimensions. */
    glm::vec3 size() const;

    /** @return the bounding box center point. */
    glm::vec3 center() const;

    /** @brief Access min or max by an integral index.
     * @param index must be 0 or 1.
     * @return min if index is 0, max otherwise.
     */
    const glm::vec3& operator[](int index) const;

    /** @brief Access min or max by an integral index.
     * @param index must be 0 or 1.
     * @return min if index is 0, max otherwise.
     */
    glm::vec3& operator[](int index);

    /** @return true if min and max match, false otherwise. */
    bool operator==(const Bounds& other) const;

    /** @returns false if min and max match, true otherwise. */
    bool operator!=(const Bounds& other) const;

    /** @brief Operator overloadings for Bounds::unite(). */
    const Bounds& operator+(const Bounds& other);
    const Bounds& operator+(const glm::vec3& point);

    alignas(16) glm::vec3 min; //!< The bounds extent's minimum.
    alignas(16) glm::vec3 max; //!< The bounds extent's maximum.
};

#include "Bounds.inl"
