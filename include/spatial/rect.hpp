//
// Created by gomkyung2 on 2023/08/11.
//

#ifndef SPATIAL_RECT_HPP
#define SPATIAL_RECT_HPP

#include "vector2.hpp"
#include "utils/thrower.hpp"
#include "utils/macros.hpp"

namespace spatial{
    template <typename T>
    class Rect{
    public:
        /////////////////////////
        // Fields.
        /////////////////////////

        const Vector2<T> position;
        const Vector2<T> size;

        /////////////////////////
        // Constructors.
        /////////////////////////

        constexpr Rect(const Vector2<T> &position, const Vector2<T> &size) NOEXCEPT_IF_RELEASE : position(position), size(size) {
#ifndef NDEBUG
            if (size.x < 0 || size.y < 0) {
                utils::throwInvalidArgument("Rect::Rect");
            }
#endif
        }

        constexpr Rect(T left, T top, T right, T bottom) NOEXCEPT_IF_RELEASE : position(left, top), size(right - left, bottom - top) {}
        constexpr Rect(const Rect&) noexcept = default;
        constexpr Rect &operator=(const Rect&) noexcept = default;

        /////////////////////////
        // Methods.
        /////////////////////////

        constexpr T left() const noexcept { return position.x; }
        constexpr T top() const noexcept { return position.y; }
        constexpr T right() const noexcept { return position.x + size.x; }
        constexpr T bottom() const noexcept { return position.y + size.y; }

        constexpr bool contains(const Vector2<T> &point) const noexcept {
            return left() <= point.x && point.x <= right() && top() <= point.y && point.y <= bottom();
        }
    };

    using IntRect = Rect<int>;
    using FloatRect = Rect<float>;
};

#endif //SPATIAL_RECT_HPP
