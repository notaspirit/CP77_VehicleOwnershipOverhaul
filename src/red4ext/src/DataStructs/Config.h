#pragma once
#include <string>
#include <unordered_map>

namespace RealisticVehicleCallSystem
{
struct Config
{
public:
    bool enabled = true;
    std::unordered_map<std::string, bool> GarageEnabledOverrides;
};
}
