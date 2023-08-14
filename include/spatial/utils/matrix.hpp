//
// Created by gomkyung2 on 2023/08/11.
//

#ifndef SPATIAL_MATRIX_HPP
#define SPATIAL_MATRIX_HPP

#include <cstddef>

#include "thrower.hpp"

namespace spatial::utils{
    template <typename T>
    class Matrix{
    private:
        /////////////////////////
        // Fields.
        /////////////////////////

        T *data;

    public:
        /////////////////////////
        // Fields.
        /////////////////////////

        const std::size_t rows;
        const std::size_t columns;

        /////////////////////////
        // Constructors and destructor.
        /////////////////////////

        constexpr Matrix(std::size_t rows, std::size_t columns) : rows(rows), columns(columns) {
            data = new T[rows * columns];
        }

        constexpr ~Matrix(){
            delete[] data;
        }

        /////////////////////////
        // Methods.
        /////////////////////////

        constexpr T &operator()(std::size_t row, std::size_t column) noexcept{
            return data[row * columns + column];
        }

        constexpr const T &operator()(std::size_t row, std::size_t column) const noexcept{
            return data[row * columns + column];
        }

        constexpr T &at(std::size_t row, std::size_t column) {
            if (row >= rows || column >= columns) {
                utils::throwOutOfRange("Matrix::at");
            }

            return data[row * columns + column];
        }

        constexpr const T &at(std::size_t row, std::size_t column) const{
            if (row >= rows || column >= columns) {
                utils::throwOutOfRange("Matrix::at");
            }

            return data[row * columns + column];
        }

//        /**
//         * @brief Find \p value in matrix and return its 2d index.
//         *
//         * @param value Value to find.
//         * @param proj Projection function for data.
//         * @return 2D index (row, column) of \p value if found, otherwise \p std::nullopt.
//         */
//        constexpr std::optional<std::array<std::size_t, 2>> find(const T &value, auto &&proj = std::identity{}) const{
//            static_assert(std::is_invocable_v<decltype(proj), const T&>, "proj must be invocable with const T&");
//            static_assert(std::equality_comparable_with<T, std::invoke_result_t<decltype(proj), const T&>>, "proj must return equality comparable type");
//
//            for (std::size_t i = 0; i < rows * columns; ++i){
//                if (value == proj(data[i])){
//                    return std::make_optional(std::array { i / columns, i % columns });
//                }
//            }
//
//            return std::nullopt;
//        }
    };
};

#endif //SPATIAL_MATRIX_HPP
