//
// Created by gomkyung2 on 2023/08/11.
//

#include <random>
#include <numbers>

#include <spatial/grid.hpp>
#include <boost/ut.hpp>

struct Body{
public:
    std::array<float, 2> position;
};

struct BodyPositionGetter{
    spatial::Vector2f operator()(const Body &body) const noexcept{
        return { body.position[0], body.position[1] };
    }
};

int main(){
    using namespace boost::ut;

    "Grid::Grid"_test = []{
#ifndef NDEBUG
        expect(throws<std::invalid_argument>([](){
            spatial::Grid<float, Body, BodyPositionGetter>(spatial::FloatRect(0, 0, 100, 100), 1, 0);
        }));
        expect(throws<std::invalid_argument>([](){
            spatial::Grid<float, Body, BodyPositionGetter>(spatial::FloatRect(0, 0, 100, 100), 0, 1);
        }));
#endif
    };

    "cellSize"_test = []{
        spatial::Grid<float, Body, BodyPositionGetter> grid(spatial::FloatRect(0, 0, 100, 100), 5, 10);
        expect(grid.cellSize() == spatial::Vector2f { 10, 20 });
    };

    "getCellIndex"_test = []{
        spatial::Grid<float, Body, BodyPositionGetter> grid(spatial::FloatRect(0, 0, 100, 100), 10, 5);
        expect(grid.getCellIndex(Body { { 0.5, 5.7 } }) == std::array<std::size_t, 2> { 0, 0 });
        expect(grid.getCellIndex(Body { { 14.4, 20.8 } }) == std::array<std::size_t, 2> { 2, 0 });
        expect(grid.getCellIndex(Body { { 85.5, 99.9 } }) == std::array<std::size_t, 2> { 9, 4 });

#ifndef NDEBUG
        expect(throws<std::out_of_range>([&grid](){
            grid.getCellIndex(Body { { 100.0, 100.0 } });
        }));
#endif
    };

    "getBodyCount"_test = []{
        spatial::Grid<float, Body, BodyPositionGetter> grid(spatial::FloatRect(0, 0, 100, 100), 10, 5);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution dis { 0.f, 100.f };

        for (int i = 0; i < 100; ++i) {
            grid.addBody(std::make_shared<Body>(std::array { dis(gen), dis(gen) }));
        }

        expect(grid.getBodyCount() == 100_i);
    };

    "addBody"_test = []{
        spatial::Grid<float, Body, BodyPositionGetter> grid(spatial::FloatRect(0, 0, 100, 100), 10, 5);

        const auto &cell1 = grid.addBody(std::make_shared<Body>(std::array { 3.f, 5.7f })); // (0, 0)
        const auto &cell2 = grid.addBody(std::make_shared<Body>(std::array { 12.f, 8.3f })); // (0, 0)
        expect(&cell1 == &cell2);

        const auto &cell3 = grid.addBody(std::make_shared<Body>(std::array { 14.4f, 20.8f })); // (2, 0)
        expect(&cell1 != &cell3);

        expect(grid.getBodyCount() == 3_i);
    };

    "removeBody"_test = []{
        // Add 100 bodies to grid and remove all.
        spatial::Grid<float, Body, BodyPositionGetter> grid(spatial::FloatRect(0, 0, 100, 100), 10, 5);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution dis { 0.f, 100.f };

        std::vector<std::shared_ptr<Body>> bodies;
        for (int i = 0; i < 100; ++i) {
            auto body = std::make_shared<Body>(std::array { dis(gen), dis(gen) });
            grid.addBody(body);

            bodies.emplace_back(std::move(body));
        }

        std::vector<std::size_t> removed_counts;
        removed_counts.reserve(100);
        for (const auto &body : bodies){
            auto &cell = grid.getBodyCell(*body);
            auto removed_count = grid.removeBody(*body, cell);

            removed_counts.push_back(removed_count);
        }

        expect(std::all_of(removed_counts.cbegin(), removed_counts.cend(), [](auto count){ return count == 1; }));
    };

    "clearAllBodies"_test = []{
        // Add 100 bodies to grid and remove all.
        spatial::Grid<float, Body, BodyPositionGetter> grid(spatial::FloatRect(0, 0, 100, 100), 10, 5);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution dis { 0.f, 100.f };

        for (int i = 0; i < 100; ++i) {
            grid.addBody(std::make_shared<Body>(std::array { dis(gen), dis(gen) }));
        }

        grid.clearAllBodies();

        expect(grid.getBodyCount() == 0_i);
    };

    "updateBodyCell"_test = []{
        spatial::Grid<float, Body, BodyPositionGetter> grid(spatial::FloatRect(0, 0, 100, 100), 10, 5);

        auto body = std::make_shared<Body>(std::array { 3.f, 5.7f });
        auto &previous_cell = grid.addBody(body); // (0, 0)

        body->position = { 14.4f, 20.8f }; // (2, 0)
        auto &current_cell = grid.updateBodyCell(*body, previous_cell);

        expect(std::distance(&previous_cell, &current_cell) == 10_i); // Grid is 10x5 -> 5 cells per row. 5 * 2 = 10.
    };

    "queryDistance"_test = []{
        spatial::Grid<float, Body, BodyPositionGetter> grid(spatial::FloatRect(0, 0, 2, 2), 2, 2);

        auto body1 = std::make_shared<Body>(std::array { 0.9f, 0.9f });
        auto cell_index1 = grid.getCellIndex(*body1);
        grid.addBody(body1);

        expect(grid.queryDistance(*body1, cell_index1, 0.5f).empty()); // self should not be included.
#ifndef NDEBUG
        expect(throws<std::invalid_argument>([&]{
            grid.queryDistance(*body1, cell_index1, 1.2f); // distance exceeds cell size.
        }));
#endif

        grid.addBody(std::make_shared<Body>(std::array { 1.1f, 0.9f })); // 2
        grid.addBody(std::make_shared<Body>(std::array { 0.9f, 1.1f })); // 3
        grid.addBody(std::make_shared<Body>(std::array { 1.1f, 1.1f })); // 4

        expect(grid.queryDistance(*body1, cell_index1, 0.1f).size() == 0_i); // nothing in distance 0.1f
        expect(grid.queryDistance(*body1, cell_index1, 0.2001f).size() == 2_i); // 2, 3 in distance 0.2001f (marginal 0.001f for floating point error)
        expect(grid.queryDistance(*body1, cell_index1, 0.3f).size() == 3_i); // 2, 3, 4 in distance 0.3f
    };

    "queryDistancePair"_test = []{
        {
            spatial::Grid<float, Body, BodyPositionGetter> grid(spatial::FloatRect(0, 0, 100, 100), 10, 10);

            for (auto i = 0; i < 10; ++i) {
                for (auto j = 0; j < 10; ++j) {
                    auto x = 10.f * static_cast<float>(j) + (j % 2 == 0 ? 9.f : 1.f);
                    auto y = 10.f * static_cast<float>(i) + (i % 2 == 0 ? 9.f : 1.f);

                    /*
                     * +----+----+----+----+----+--
                     * |   .|.   |   .|.   |   .|
                     * +----+----+----+----+----+--
                     * |   `|`   |   `|`   |   `|
                     * +----+----+----+----+----+--
                     * |   .|.   |   .|.   |   .|
                     * +----+----+----+----+----+--
                     * |   `|`   |   `|`   |   `|
                     * +----+----+----+----+----+--
                     * |   .|.   |   .|.   |   .|
                     * +----+----+----+----+----+--
                     * |   `|`   |   `|`   |   `|
                     */

                    grid.addBody(std::make_shared<Body>(std::array { x, y }));
                }
            }

            expect(grid.queryDistancePair(1.f).size() == 0_i);
            expect(grid.queryDistancePair(2.001f).size() == 100_i); // 4 * 5 * 5 = 100
            expect(grid.queryDistancePair(3.f).size() == 150_i); // C(4, 2) * 5 * 5 = 150
        }

        {
            spatial::Grid<float, Body, BodyPositionGetter> grid(spatial::FloatRect(0, 0, 100, 100), 10, 10);

            constexpr float RADIUS = 4.f; // The farthest distance between two bodies in the grid is 8.f < 10.f = cell size.
            for (auto i = 0; i < 100; ++i) {
                auto theta = (2.f * std::numbers::pi_v<float>) * (static_cast<float>(i) / 100.f);
                auto x = 20.f + RADIUS * std::cos(theta);
                auto y = 20.f + RADIUS * std::sin(theta);

                grid.addBody(std::make_shared<Body>(std::array { x, y }));
            }

            // Now bodies are distributed in cell (1, 1), (1, 2), (2, 1), and (2, 2) in circular shape.
            // The distance between the nearest bodies = 4 * sqrt(2*(1-cos(2*pi/100))) = 0.2512

            expect(grid.queryDistancePair(0.26f).size() == 100_i);
            expect(grid.queryDistancePair(8.001f).size() == 4950_i); // C(100, 2) = 4950
        }
    };
}