// grid2d.hpp
#pragma once
#include <cstddef>
#include <vector>

template <typename T>
class Grid2D {
public:
    Grid2D(std::size_t rows, std::size_t cols, const T& init = T{})
        : m_rows(rows), m_cols(cols), m_data(rows * cols, init) {}

    T& operator()(std::size_t row, std::size_t col) {
        return m_data[row * m_cols + col];
    }
    const T& operator()(std::size_t row, std::size_t col) const {
        return m_data[row * m_cols + col];
    }

    std::size_t rows() const { return m_rows; }
    std::size_t cols() const { return m_cols; }

private:
    std::size_t m_rows;
    std::size_t m_cols;
    std::vector<T> m_data;
};