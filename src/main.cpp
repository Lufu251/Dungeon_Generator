#include <iostream>
#include <queue>

#include <raylib.h>
#include <dungeon.hpp>
#include <procgen/dungeon_generator.hpp>
#include <helpers/rng.hpp>

void draw_level(Level& level, Vector2 position, int tile_size){
    for (size_t x = 0; x < level.m_grid.cols(); x++){
        for (size_t y = 0; y < level.m_grid.rows(); y++){
            DrawRectangle(x * tile_size + position.x, y * tile_size + position.y, tile_size, tile_size, level.m_grid(x,y).m_color);
        }
    }
}

int main(void){
    // Initialization
    const int screenWidth = 800;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "dungeon_generator");

    SetTargetFPS(60);

    Rng rng(123123);
    Level level(30,30);
    Grid2D<Tile> grid(30,30, Tile({200,0,0, 255}));
    DungeonGenerator dg;

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        if(IsKeyPressed(KEY_A)) {
            dg.generateRooms(60, 15, 25, 50, rng);
        } 

        if(IsKeyPressed(KEY_S)) {
            dg.separateRooms(150);
        } 
        
        if(IsKeyPressed(KEY_D)) {
            dg.selectRandomRooms(20, rng);
        } 

        if(IsKeyPressed(KEY_F)) {
            dg.buildEdges();
        }

        // Draw
        BeginDrawing();
            ClearBackground(GRAY);

            dg.drawRooms({400,400}, 2);
            dg.drawEdges({400,400}, 2, YELLOW);
        EndDrawing();
    }

    // De-Initialization
    CloseWindow(); // Close window and OpenGL context

    return 0;
}