#pragma once

#include "MinHook.h"
#include "RED4ext/RTTISystem.hpp"
#include "RED4ext/Scripting/IScriptable.hpp"
#include "RED4ext/Scripting/Stack.hpp"
#include "RED4ext/Scripting/Natives/Quaternion.hpp"
#include "RED4ext/Scripting/Natives/Vector4.hpp"
#include "RED4ext/Scripting/Natives/Generated/WorldTransform.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/VehicleSystem.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/data/VehicleType.hpp"

namespace RealisticVehicleCallSystem
{

class RealisticVehicleCallSystemNative : RED4ext::IScriptable
{
public:
    static void Hook();
    static char __fastcall hkFindSpawnLocation(RED4ext::gameVehicleSystem *vehicleSystem, RED4ext::Vector3 *playerPosition, RED4ext::Vector3 *outPosition, RED4ext::
                                               Quaternion *playerAndOutRotation);
    static bool __fastcall hkSpawnPlayerVehicle(RED4ext::gameVehicleSystem* vehicleSystem, RED4ext::gamedataVehicleType vehicleType, RED4ext::TweakDBID vehicleID, bool spawnOnlyOnValidRoad);
    static uint64_t __fastcall hkSummonVehicle(long long param_1, uint32_t *param_2, uint64_t param_3, uint64_t param_4, uint64_t param_5, char param_6, uint8_t
                                               param_7, uint32_t param_8, uint8_t param_9, void *param_10);

    static void hkPreSpawnPlayerVehicleRTTI(RED4ext::IScriptable *aContext, RED4ext::CStackFrame *aFrame, RED4ext::CString *aOut, int64_t a4);
    static void hkPreFindSpawnLocationRTTI(RED4ext::IScriptable *aContext, RED4ext::CStackFrame *aFrame, RED4ext::WorldTransform *aOut, int64_t a4);
    static void hkPreSummonVehicleRTTI(RED4ext::IScriptable *aContext, RED4ext::CStackFrame *aFrame, RED4ext::CString *aOut, int64_t a4);

    static void hkPostSpawnPlayerVehicleRTTI(RED4ext::IScriptable *aContext, RED4ext::CStackFrame *aFrame, RED4ext::CString *aOut, int64_t a4);

    RED4ext::CClass* GetNativeType();
};

constexpr const char* ToString(const RED4ext::game::data::VehicleType type)
{
    switch (type)
    {
        case RED4ext::game::data::VehicleType::Bike:
            return "Bike";

        case RED4ext::game::data::VehicleType::Car:
            return "Car";

        case RED4ext::game::data::VehicleType::Panzer:
            return "Panzer";

        case RED4ext::game::data::VehicleType::Count:
            return "Count";

        case RED4ext::game::data::VehicleType::Invalid:
            return "Invalid";

        default:
            return "Unknown";
    }
}

}
