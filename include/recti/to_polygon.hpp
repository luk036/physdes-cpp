#pragma once

#include <vector>

#include "polygon.hpp"
#include "rpolygon.hpp"

namespace recti {

    /**
     * @brief Converts a rectilinear polygon to a general polygon
     *
     * This function adds intermediate points to ensure the polygon remains rectilinear
     * when converted to a general polygon representation.
     *
     * @tparam T The type of the coordinates
     * @param rpoly The rectilinear polygon to convert
     * @return A Polygon object representing the converted polygon
     */
    template <typename T>
    constexpr auto to_polygon(const RPolygon<T>& rpoly) -> Polygon<T> {
        const auto& _vecs = rpoly.vectors();
        if (_vecs.empty()) {
            return Polygon<T>(rpoly.origin(), {});
        }

        std::vector<Vector2<T>> new_vecs;
        Vector2<T> current_point(0, 0);

        for (const auto& next_point : _vecs) {
            if (current_point.x() != next_point.x() && current_point.y() != next_point.y()) {
                // Add intermediate point for non-rectilinear segment
                new_vecs.emplace_back(next_point.x(), current_point.y());
            }
            new_vecs.push_back(next_point);
            current_point = next_point;
        }

        // Handle closing segment
        Vector2<T> first_point(0, 0);
        if (current_point.x() != first_point.x() && current_point.y() != first_point.y()) {
            new_vecs.emplace_back(first_point.x(), current_point.y());
        }

        return Polygon<T>(rpoly.origin(), std::move(new_vecs));
    }
}  // namespace recti
