#pragma once
#include <numeric>

namespace RealisticVehicleCallSystem
{
template<typename Container>
float Average(const Container& values)
{
    if (values.empty())
        return 0.0f;

    return std::accumulate(values.begin(), values.end(), 0.0f)
           / static_cast<float>(values.size());
}
}
