#pragma once

#include <vector>
#include <cmath>
#include <numbers>
#include <algorithm>

#include <delabella.h>
#include <raylib.h>
#include <raymath.h>
#include <helpers/rng.hpp>

struct DungeonGenerator {
    struct Edge {
        int a;
        int b;

        auto operator<=>(const Edge&) const = default;  // gives ==, <, etc. for sort/unique
    };

    std::vector<Rectangle> m_rooms;
    std::vector<Edge> m_edges;

    DungeonGenerator(){}

    // -------------------- Helpers --------------------
    // Random point inside a circle centered on the origin
    Vector2 getRandomPointInCircle(float radius, Rng& rng) {
        float t = 2.0f * std::numbers::pi_v<float> * rng.nextFloat();
        float r = std::sqrt(rng.nextFloat());
        return { radius * r * std::cos(t), radius * r * std::sin(t) };
    }

    Vector2 roomCenter(size_t i) const {
        const Rectangle& r = m_rooms[i];
        return { r.x + r.width * 0.5f, r.y + r.height * 0.5f };
    }

    // -------------------- Generation --------------------

    // Generate random amount of rooms inside of a circle 
    void generateRooms(int room_count, int min_room_size, int max_room_size, float radius, Rng& rng){
        m_rooms.clear();
        m_edges.clear();
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
        m_edges.clear();

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

        m_edges.clear();

        for (size_t i = 0; i < room_count; i++) {
            size_t r = rng.nextInt(i, m_rooms.size() - 1);
            std::swap(m_rooms[i], m_rooms[r]);
        }

        m_rooms.resize(room_count);
    }

    // Triangulates the room centers. Fills `triangles` with 3 room indices per triangle.
    void triangulateRooms(std::vector<int>& triangles) {
        triangles.clear();

        const size_t n = m_rooms.size();
        if (n < 3) return;

        // Vector2 is {float x, y}, so this array is already interleaved: x0, y0, x1, y1, ...
        std::vector<Vector2> pts(n);
        for (size_t i = 0; i < n; i++) pts[i] = roomCenter(i);

        IDelaBella2<float>* idb = IDelaBella2<float>::Create();

        if (idb->Triangulate((int)n, &pts[0].x, &pts[0].y, sizeof(Vector2)) > 0) {
            const int tri_count = idb->GetNumOutputIndices() / 3;
            triangles.reserve(tri_count * 3);

            const IDelaBella2<float>::Simplex* s = idb->GetFirstDelaunaySimplex();

            for (int t = 0; t < tri_count; t++, s = s->next)
                for (int k = 0; k < 3; k++)
                    triangles.push_back(s->v[k]->i);
        }

        idb->Destroy();
    }

    // Triangulates the rooms and builds the unique edge list.
    size_t buildEdges() {
        std::vector<int> triangles;
        triangulateRooms(triangles);

        m_edges.clear();
        m_edges.reserve(triangles.size());   // 3 edges per triangle = same count as indices

        for (size_t t = 0; t + 2 < triangles.size(); t += 3) {
            for (size_t k = 0; k < 3; k++) {
                const int u = triangles[t + k];
                const int v = triangles[t + (k + 1) % 3];
                m_edges.push_back({std::min(u, v), std::max(u, v)});
            }
        }

        std::sort(m_edges.begin(), m_edges.end());
        m_edges.erase(std::unique(m_edges.begin(), m_edges.end()), m_edges.end());
        return m_edges.size();
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

    // Debug: draws every unique edge between room centers
    void drawEdges(Vector2 offset, int tile_size, Color color){
        for (const Edge& e : m_edges){
            Vector2 pa = Vector2Add(Vector2Scale(roomCenter(e.a), (float)tile_size), offset);
            Vector2 pb = Vector2Add(Vector2Scale(roomCenter(e.b), (float)tile_size), offset);
            DrawLineV(pa, pb, color);
        }
    }
};
