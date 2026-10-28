#pragma once
#include <cstdint>
#include <random>

class Rng {
public:
    explicit Rng(std::uint32_t seed) : engine_(seed) {}

    void seed(std::uint32_t seed) { engine_.seed(seed); }

    // Integer in [min, max], both inclusive
    int nextInt(int min, int max) {
        std::uint32_t range = static_cast<std::uint32_t>(max - min) + 1u;
        return min + static_cast<int>(engine_() % range);
    }

    // Float in [min, max], both inclusive
    float nextFloat(float min, float max) {
        return min + nextFloat() * (max - min);
    }

    // Float in [0, 1)
    float nextFloat() {
        return (engine_() >> 8) * (1.0f / 16777216.0f);  // 24 bits -> exact float
    }

private:
    std::mt19937 engine_;
};