#pragma once
#include "DataStructs/Garage.h"

namespace RealisticVehicleCallSystem
{
class GarageLoader {
public:
    static std::vector<RealisticVehicleSystem::Garage> LoadGarages();
};
}

