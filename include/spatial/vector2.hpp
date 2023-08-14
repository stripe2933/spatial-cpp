//
// Created by gomkyung2 on 2023/08/11.
//

#ifndef SPATIAL_VECTOR2_HPP
#define SPATIAL_VECTOR2_HPP

#include <cmath>

#include "utils/thrower.hpp"
#include "utils/macros.hpp"

namespace spatial{
    template <typename T>
    struct Vector2{
        /////////////////////////
        /// Fields.
        /////////////////////////

        T x;
        T y;

        /////////////////////////
        // Constructors.
        /////////////////////////

        constexpr Vector2(T x, T y) noexcept : x(x), y(y) {}
        constexpr Vector2(const Vector2&) noexcept = default;
        constexpr Vector2 &operator=(const Vector2&) noexcept = default;

        /////////////////////////
        // Arithmetic operators.
        /////////////////////////

        constexpr Vector2 operator+(const Vector2 &other) const noexcept { return { x + other.x, y + other.y }; }
        constexpr Vector2 &operator+=(const Vector2 &other) noexcept { x += other.x; y += other.y; return *this; }
        constexpr Vector2 operator-(const Vector2 &other) const noexcept { return { x - other.x, y - other.y }; }
        constexpr Vector2 &operator-=(const Vector2 &other) noexcept { x -= other.x; y -= other.y; return *this; }
        constexpr Vector2 operator*(T scalar) const noexcept { return { x * scalar, y * scalar }; }
        constexpr Vector2 &operator*=(T scalar) noexcept { x *= scalar; y *= scalar; return *this; }
        friend constexpr Vector2 operator*(T scalar, const Vector2 &self) noexcept { return self * scalar; }

        /////////////////////////
        // Comparison operators.
        /////////////////////////

        constexpr bool operator==(const Vector2 &other) const noexcept { return x == other.x && y == other.y; }
        constexpr bool operator!=(const Vector2 &other) const noexcept { return !(*this == other); }

        /////////////////////////
        // Methods.
        /////////////////////////

        /**
         * @brief Multiply two vector by their components.
         * @param other Rhs vector.
         * @return Elementwise multiplied vector.
         */
        constexpr Vector2 cwiseMul(const Vector2 &other) const noexcept {
            return { x * other.x, y * other.y };
        }

        /**
         * @brief Divide two vector by their components.
         * @param other Rhs vector.
         * @return Elementwise divided vector.
         * @throw std::invalid_argument If any component of \p other is zero in debug mode.
         */
        constexpr Vector2 cwiseDiv(const Vector2 &other) const NOEXCEPT_IF_RELEASE {
#ifndef NDEBUG
            if (other.x == 0 || other.y == 0) {
                utils::throwInvalidArgument("other must not have zero component");
            }
#endif
            return { x / other.x, y / other.y };
        }

        /**
         * @brief Dot product of two vectors.
         * @param other Rhs vector.
         * @return Dot product of two vectors.
         */
        constexpr T dot(const Vector2 &other) const noexcept {
            return x * other.x + y * other.y;
        }

        /**
         * Get distance of two vector, i.e. ||other - this||.
         * @param other Rhs vector.
         * @return Distance of two vector.
         * @note Since it uses square root calculation, it is more expensive than \p distance2.
         * If you can, use \p distance2 instead.
         */
        constexpr T distance(const Vector2 &other) const noexcept {
            return std::hypot(other.x - x, other.y - y);
        }

        /**
         * Get square of distance of two vector, i.e. ||other - this||^2.
         * @param other Rhs vector.
         * @return Square of distance of two vector.
         * @note It is more efficient than \p distance. Use this if you can.
         */
        constexpr T distance2(const Vector2 &other) const noexcept {
            return std::pow(other.x - x, 2) + std::pow(other.y - y, 2);
        }
    };

    using Vector2i = Vector2<int>;
    using Vector2u = Vector2<unsigned int>;
    using Vector2f = Vector2<float>;
};

#endif //SPATIAL_VECTOR2_HPP
