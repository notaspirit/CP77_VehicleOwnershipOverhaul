#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "TweakDBIDHasher.h"
#include "RED4ext/NativeTypes.hpp"
#include "RED4ext/Scripting/Natives/Quaternion.hpp"
#include "RED4ext/Scripting/Natives/Vector3.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/data/VehicleType.hpp"

namespace RealisticVehicleSystem {
    struct Slot
    {
    public:
        std::unordered_map<RED4ext::gamedataVehicleType, bool> VehicleTypes;
        std::unordered_set<RED4ext::TweakDBID, RealisticVehicleCallSystem::TweakDBIDHasher> WhiteListedVehicles;
        std::unordered_set<RED4ext::TweakDBID, RealisticVehicleCallSystem::TweakDBIDHasher> BlackListedVehicles;
        RED4ext::Vector3 Position;
        RED4ext::Quaternion Rotation;
    };
}
