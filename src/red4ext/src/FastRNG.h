#pragma once
#include <cstdint>

namespace RealisticVehicleCallSystem {
struct FastRNG
{
public:
    uint32_t state;
    void xorshift32();
    uint32_t getInt32(uint32_t max, uint32_t min = 0);
    float getFloat(float max, float min = 0);
};
}
