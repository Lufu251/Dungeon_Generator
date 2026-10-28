#pragma once

#include <vector>
#include <raylib.h>

#include <helpers/grid2d.hpp>

struct Tile{
    Color m_color;
    Tile(const Color& color) : m_color(color){}
};

struct Room{
    int m_width;
    int m_height;
    Room(int width, int height) : m_width(width), m_height(height){}
};

struct Level{
    Grid2D<Tile> m_grid;
    Level(int x, int y) : m_grid(x, y, WHITE){}
};
