#pragma once

#include <type_traits>
#include <memory>

/**
 * @brief A creator-pattern implementation using CRTP. Used in FunctionClass<..., ...>.
 * @tparam Self The deriving class itself.
 */
template <typename Self>
struct Creatable
{
    /**
     * @brief Constructs a shared_ptr of the type given as the template parameter.
     * Calls std::make_shared<Self>(...);
     */
    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<Self, Args...>>>
    static std::shared_ptr<Self> create(Args&&... args)
    {
        return std::make_shared<Self>(std::forward<Args&&>(args)...);
    }
};
