#pragma once

#include <cstddef>

struct Point {
    int column{};
    int row{};

    bool operator==(const Point& other) const {
        return column == other.column && row == other.row;
    }

    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

struct PointHash {
    std::size_t operator()(const Point& point) const noexcept {
        return (static_cast<std::size_t>(point.column) << 32U) ^
               static_cast<std::size_t>(point.row);
    }
};

