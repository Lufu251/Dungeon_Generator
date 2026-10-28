#pragma once

#include <vector>
#include <cmath>
#include <numbers>

#include <delabella.h>
#include <raylib.h>
#include <raymath.h>
#include <helpers/rng.hpp>

struct DungeonGenerator {
    std::vector<Rectangle> m_rooms;
    std::vector<int> m_triangles;

    DungeonGenerator(){}


    // -------------------- Helpers --------------------
    // Random point inside a circle centered on the origin
    // Random point inside a circle centered on the origin (uniform by area)
Vector2 getRandomPointInCircle(float radius, Rng& rng) {
    float t = 2.0f * std::numbers::pi_v<float> * rng.nextFloat();
    float r = std::sqrt(rng.nextFloat());
    return { radius * r * std::cos(t), radius * r * std::sin(t) };
}

    // -------------------- Generation --------------------

    // Generate random amount of rooms inside of a circle 
    void generateRooms(int room_count, int min_room_size, int max_room_size, float radius, Rng& rng){
        m_rooms.clear();
        m_triangles.clear();
        m_rooms.reserve(room_count);

        for (int i = 0; i < room_count; i++){
            Vector2 pos = getRandomPointInCircle(radius, rng);

            m_rooms.push_back({
                std::round(pos.x), std::round(pos.y),
                std::round(rng.nextFloat(min_room_size, max_room_size)),
                std::round(rng.nextFloat(min_room_size, max_room_size))});
        }
    }

    // One pass over all room pairs. Returns true if any room was moved.
    bool separationPass(){
        bool moved = false; // 

        for (size_t i = 0; i < m_rooms.size(); i++){
            for (size_t j = i + 1; j < m_rooms.size(); j++){

                Rectangle& a = m_rooms[i];
                Rectangle& b = m_rooms[j];

                // Calculate penetration
                const float pushLeftX = a.x + a.width - b.x;
                const float pushRightX = b.x + b.width - a.x;
                const float pushUpY = a.y + a.height - b.y;
                const float pushDownY = b.y + b.height - a.y;

                // Collision is only true if all four values are positive
                if(std::min({pushLeftX, pushRightX, pushUpY, pushDownY}) <= 0.0f) continue;

                // In which direction should be pushed to solve the collision
                const float pushX = (pushLeftX < pushRightX) ? -pushLeftX : pushRightX;
                const float pushY = (pushUpY < pushDownY) ? -pushUpY : pushDownY;

                // Push both rectangles half the distance away along the shorter axis
                if (std::abs(pushX) < std::abs(pushY)) {
                    a.x += std::round(pushX * 0.5f);
                    b.x -= std::round(pushX * 0.5f);
                } else {
                    a.y += std::round(pushY * 0.5f);
                    b.y -= std::round(pushY * 0.5f);
                }

                moved = true;
            }
        }
        return moved;
    }

    bool separateRooms(size_t max_iterations){
        for (size_t i = 0; i < max_iterations; i++) {
            if(!separationPass()){
                return true;
                break;
            }
        }
        return false;
    }

    void selectRandomRooms(size_t room_count, Rng& rng) {
        if (room_count >= m_rooms.size()) return; // nothing to cut

        m_triangles.clear();

        for (size_t i = 0; i < room_count; i++) {
            size_t r = rng.nextInt(i, m_rooms.size() - 1);
            std::swap(m_rooms[i], m_rooms[r]);
        }

        m_rooms.resize(room_count);
    }

    // Triangulates the room centers. Returns the number of triangles, 0 on failure.
    size_t triangulateRooms() {
        m_triangles.clear();

        const size_t n = m_rooms.size();
        if (n < 3) return 0;

        // Room centers interleaved: x0, y0, x1, y1, ... (index i == room i)
        std::vector<float> pts(n * 2);
        for (size_t i = 0; i < n; i++) {
            pts[2*i]     = m_rooms[i].x + m_rooms[i].width  * 0.5f;
            pts[2*i + 1] = m_rooms[i].y + m_rooms[i].height * 0.5f;
        }

        IDelaBella2<float>* idb = IDelaBella2<float>::Create();
        const int result = idb->Triangulate((int)n, pts.data(), pts.data() + 1, 2 * sizeof(float));

        size_t tri_count = 0;
        if (result > 0) {
            tri_count = (size_t)(idb->GetNumOutputIndices() / 3);
            m_triangles.reserve(tri_count * 3);

            const IDelaBella2<float>::Simplex* s = idb->GetFirstDelaunaySimplex();
            for (size_t t = 0; t < tri_count && s != nullptr; t++, s = s->next) {
                m_triangles.push_back(s->v[0]->i);   // index of the original point == room index
                m_triangles.push_back(s->v[1]->i);
                m_triangles.push_back(s->v[2]->i);
            }
        }

        idb->Destroy();
        return tri_count;
    }

    // Debug
    void drawRooms(Vector2 offset, int tile_size){
        for (Rectangle& room : m_rooms){
            int x = (room.x * tile_size) + offset.x;
            int y = (room.y * tile_size) + offset.y;
            int w = room.width * tile_size;
            int h = room.height * tile_size;

            DrawRectangle(x, y, w, h, {200,0,0,255});
            DrawRectangleLines(x, y, w, h, {150,0,0,255});
        }
    }

    // Debug: draws every triangle edge between room centers
    void drawTriangles(Vector2 offset, int tile_size, Color color){
        for (size_t t = 0; t + 2 < m_triangles.size(); t += 3){
            Vector2 p[3];
            for (int k = 0; k < 3; k++){
                const Rectangle& room = m_rooms[m_triangles[t + k]];
                p[k] = {
                    (room.x + room.width  * 0.5f) * tile_size + offset.x,
                    (room.y + room.height * 0.5f) * tile_size + offset.y
                };
            }
            DrawLineV(p[0], p[1], color);
            DrawLineV(p[1], p[2], color);
            DrawLineV(p[2], p[0], color);
        }
    }
};
