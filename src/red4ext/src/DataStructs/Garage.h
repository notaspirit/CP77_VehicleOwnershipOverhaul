#pragma once
#include <string>
#include <vector>

#include "Slot.h"
#include "RED4ext/Scripting/Natives/Vector3.hpp"

namespace RealisticVehicleSystem
{
    struct Garage {
    public:
        std::string Name;
        std::string QuestFact;
        RED4ext::Vector3 Position;
        std::unordered_map<RED4ext::gamedataVehicleType, bool> VehicleTypes;
        std::unordered_set<RED4ext::TweakDBID, RealisticVehicleCallSystem::TweakDBIDHasher> WhiteListedVehicles;
        std::unordered_set<RED4ext::TweakDBID, RealisticVehicleCallSystem::TweakDBIDHasher> BlackListedVehicles;
        std::vector<Slot> Slots;
    };
}
