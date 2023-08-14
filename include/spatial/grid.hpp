#ifndef SPATIAL_GRID_HPP
#define SPATIAL_GRID_HPP

#include <concepts>
#include <list>
#include <ranges>
#include <unordered_set>

#include "rect.hpp"
#include "utils/matrix.hpp"
#include "utils/thrower.hpp"
#include "utils/macros.hpp"

namespace spatial{
    template <std::floating_point T, typename Body, typename PositionGetter>
    requires std::invocable<PositionGetter, const Body&> &&
             std::is_same_v<std::invoke_result_t<PositionGetter, const Body&>, Vector2<T>>
    class Grid{
    private:
        using body_ptr_t = std::shared_ptr<Body>;
        using cell_t = std::list<body_ptr_t>;

        utils::Matrix<cell_t> cells;
        std::size_t num_bodies = 0;

        struct symmetric_pair_hash{
            constexpr std::size_t operator()(std::span<const body_ptr_t, 2> pair) const noexcept{
                return std::hash<body_ptr_t>()(pair[0]) ^ std::hash<body_ptr_t>()(pair[1]);
            }
        };

        struct symmetric_pair_equal{
            constexpr bool operator()(std::span<const body_ptr_t, 2> pair1, std::span<const body_ptr_t, 2> pair2) const noexcept{
                return (pair1[0] == pair2[0] && pair1[1] == pair2[1]) || (pair1[0] == pair2[1] && pair1[1] == pair2[0]);
            }
        };

    public:
        const Rect<T> bound;
        const std::size_t rows;
        const std::size_t columns;

        Grid(const Rect<T> &bound, std::size_t rows, std::size_t columns) : bound(bound), rows(rows), columns(columns), cells(rows, columns) {
#ifndef NDEBUG
            if (rows == 0 || columns == 0) {
                utils::throwInvalidArgument("Grid::Grid: rows and columns must be greater than 0");
            }
#endif
        }
        Grid(Grid&&) noexcept = default;

        /**
         * @brief Get cell size of grid.
         *
         * @return Cell size in vec2 form (x=width, y=height).
         */
        Vector2<T> cellSize() const NOEXCEPT_IF_RELEASE {
            return bound.size.cwiseDiv(Vector2<T> { static_cast<T>(columns), static_cast<T>(rows) });
        }

        /**
         * @brief Get cell index of body.
         *
         * @param body body to get cell index.
         * @return Cell index in std::array form (row, col).
         * @throw std::out_of_range If \p body is out of bound in debug mode.
         */
        std::array<std::size_t, 2> getCellIndex(const Body &body) const NOEXCEPT_IF_RELEASE{
            const auto relative_position = PositionGetter()(body) - bound.position;
            const auto cell_size = cellSize();

            const auto row = static_cast<std::size_t>(relative_position.y / cell_size.y);
            const auto col = static_cast<std::size_t>(relative_position.x / cell_size.x);

#ifndef NDEBUG
            if (row >= rows || col >= columns) {
                utils::throwOutOfRange("Grid::getCellIndex: out of range");
            }
#endif
            return { row, col };
        }

        /**
         * @brief Get cell that body is in.
         *
         * @param body Body to get cell.
         * @return Cell that body is in.
         */
        cell_t &getBodyCell(const Body &body) NOEXCEPT_IF_RELEASE{
            const auto [row, col] = getCellIndex(body);
            return cells(row, col);
        }

        /**
         * @brief Get number of bodies in grid.
         * @return Number of bodies in grid.
         */
        [[nodiscard]] std::size_t getBodyCount() const noexcept{
            return num_bodies;
        }

        /**
         * @brief Add body to grid.
         *
         * @param body body to add.
         * @return Reference to cell that body is added.
         */
        cell_t &addBody(auto &&body){
            static_assert(std::is_convertible_v<decltype(body), body_ptr_t>);

            auto &cell = getBodyCell(*body);
            cell.emplace_back(std::forward<decltype(body)>(body));

            num_bodies++;

            return cell;
        }

        /**
         * @brief Remove body from grid.
         *
         * @param body Body to remove.
         * @return Number of bodies removed.
         */
        std::size_t removeBody(const Body &body, cell_t &body_cell) noexcept{
            auto removed_count = body_cell.remove_if([&body](const auto &ptr){ return ptr.get() == &body; });
            num_bodies -= removed_count;

            return removed_count;
        }

        /**
         * @brief Clear all bodies in grid.
         */
        void clearAllBodies() noexcept{
            for (auto i = 0; i < rows; ++i){
                for (auto j = 0; j < columns; ++j){
                    cells(i, j).clear();
                }
            }
            num_bodies = 0;
        }

        /**
         * @brief Update body's cell when its position is changed.
         *
         * @param body Body to update.
         * @param previous_cell The cell that body was in (can be obtained by \p getBodyCell method before update).
         * @return New cell which contains the body.
         * @throw std::out_of_range If \p previous_cell does not contain \p body in debug mode.
         */
        cell_t &updateBodyCell(const Body &body, cell_t &previous_cell){
            const auto [row, col] = getCellIndex(body);
            auto &new_cell = cells(row, col);

            if (&new_cell == &previous_cell) {
                return new_cell;
            }

            auto ptr = std::find_if(previous_cell.begin(), previous_cell.end(),
                                    [&](const auto &ptr){ return ptr.get() == &body; });
#ifndef NDEBUG
            if (ptr == previous_cell.end()) {
                utils::throwOutOfRange("Grid::updateBodyCell: body not found");
            }
#endif

            new_cell.splice(new_cell.end(), previous_cell, ptr);
            return new_cell;
        }

        /**
         * @brief Get bodies in grid that distance from \p body is less than \p distance.
         *
         * @param body Body to query.
         * @param body_cell_index Cell index of \p body.
         * @param distance Distance to query.
         * @return A vector of all bodies distance less than \p distance.
         * @throw std::invalid_argument If \p distance is greater than cell size in debug mode.
         */
        std::vector<std::shared_ptr<Body>> queryDistance(const Body &body, std::array<std::size_t, 2> body_cell_index, T distance){
#ifndef NDEBUG
            auto [cell_x, cell_y] = cellSize();
            if (distance > std::min(cell_x, cell_y)){
                utils::throwInvalidArgument("Only distance smaller than or equal to cell size is supported.");
            }
#endif

            const auto body_position = PositionGetter()(body);
            const auto distance_square = std::pow(distance, 2);

            const auto is_nearby = [&](const Body &other){
                return PositionGetter()(other).distance2(body_position) <= distance_square;
            };

            // Result vector.
            std::vector<std::shared_ptr<Body>> result;

            // Find bodies in same cell.
            auto queried_body_in_same_cell = cells(body_cell_index[0], body_cell_index[1])
                     | std::views::filter([&](const auto &ptr){
                         return ptr.get() != &body && // except body itself
                                is_nearby(*ptr);
                     });

            // Insert bodies in same cell to result.
            result.insert(result.end(), queried_body_in_same_cell.begin(), queried_body_in_same_cell.end());

            // Find bodies in adjacent cells.
            constexpr auto adjacent_offsets = std::array<std::array<int, 2>, 8>{ std::array
                { -1, -1 }, { -1, 0 }, { -1, 1 },
                { 0, -1 },             {  0, 1 },
                {  1, -1 }, {  1, 0 }, {  1, 1 }
            };

            const auto translate_cell = [&](std::array<int, 2> xy) -> std::array<int, 2>{
                const auto [center_row, center_column] = body_cell_index;
                const auto [dx, dy] = xy;

                return { static_cast<int>(center_row) + dy, static_cast<int>(center_column) + dx };

            };
            const auto is_cell_within_bound = [&](std::array<int, 2> cell_index){
                auto [row, column] = cell_index;
                return row >= 0 && row < rows && column >= 0 && column < columns;
            };

            auto queried_body_in_adjacent_cell = adjacent_offsets
                    | std::views::transform(translate_cell) // adjacent offsets to cell index.
                    | std::views::filter(is_cell_within_bound) // filter only cells within bound.
                    | std::views::transform([&](auto &&cell_index) -> cell_t& {
                        return cells(cell_index[0], cell_index[1]);
                    }) // transform cell index to cell.
                    | std::views::transform([&](auto &cell){
                        return cell | std::views::filter([&](const auto &ptr) { return is_nearby(*ptr); });
                    }); // for each cell, filter bodies that are nearby.

            // Insert bodies in adjacent cells to result.
            for (auto bodies : queried_body_in_adjacent_cell){
                result.insert(result.end(), bodies.begin(), bodies.end());
            }

            return result;
        }

        /**
         * @brief Get all body pairs that distance between them is less than \p distance.
         * @param distance Distance to query.
         * @return Set of body pairs that distance between them is less than \p distance. It contains unique pairs only,
         * which means if (body1, body2) is in set, (body2, body1) is not in set.
         * @throw std::invalid_argument If \p distance is greater than cell size in debug mode.
         */
        std::unordered_set<std::array<body_ptr_t, 2>, symmetric_pair_hash, symmetric_pair_equal> queryDistancePair(T distance){
#ifndef NDEBUG
            auto [cell_x, cell_y] = cellSize();
            if (distance > std::min(cell_x, cell_y)){
                utils::throwInvalidArgument("Only distance smaller than or equal to cell size is supported.");
            }
#endif
            const auto is_nearby = [&](const Body &body1, const Body &body2){
                return PositionGetter()(body1).distance2(PositionGetter()(body2)) <= std::pow(distance, 2);
            };

            /*
             * +----+----+----+ Left figure is the portion of grid cells. This algorithm finds collision pair of cell (5)
             * |(1) |(2) |(3) | in (1) (top-left), (2) (top), (4) (left) and (5) (current). It does not find in other
             * +----+----+----+ adjacent cells, like (3) because it will be checked by (6). However, if the body pair is
             * |(4) |(5) |(6) | within edge of (2) and (5), they will be checked twice when current cell is in (6).
             * +----+----+----+ Therefore, the result should be unordered_set, which only allows unique elements.
             * |(7) |(8) |(8) | A pair is same if their elements are same, regardless of the order, i.e. (body1, body2)
             * +----+----+----+ and (body2, body1) are same. Therefore, symmetric_pair_hash and symmetric_pair_equal are used.
             */
            std::unordered_set<std::array<body_ptr_t, 2>, symmetric_pair_hash, symmetric_pair_equal> result;
            for (std::size_t row = 1; row < rows; ++row) {
                for (std::size_t col = 1; col < columns; col++) {
                    const auto &cell_current = cells(row, col);
                    const auto &cell_left = cells(row, col - 1);
                    const auto &cell_top = cells(row - 1, col);
                    const auto &cell_top_left = cells(row - 1, col - 1);

                    auto reserve_size = cell_current.size() + cell_left.size() + cell_top.size() + cell_top_left.size();
                    if (reserve_size == 0){ // Four cells are all empty
                        continue;
                    }

                    std::vector<body_ptr_t> check_bodies;
                    check_bodies.reserve(reserve_size);
                    check_bodies.insert(check_bodies.end(), cell_current.begin(), cell_current.end());
                    check_bodies.insert(check_bodies.end(), cell_left.begin(), cell_left.end());
                    check_bodies.insert(check_bodies.end(), cell_top.begin(), cell_top.end());
                    check_bodies.insert(check_bodies.end(), cell_top_left.begin(), cell_top_left.end());

                    for (auto i = 0; i < check_bodies.size(); ++i){
                        for (auto j = i + 1; j < check_bodies.size(); ++j){
                            if (is_nearby(*check_bodies[i], *check_bodies[j])){
                                result.emplace(std::array { check_bodies[i], check_bodies[j] });
                            }
                        }
                    }
                }
            }

            return result;
        }
    };
};

#endif //SPATIAL_GRID_HPP
