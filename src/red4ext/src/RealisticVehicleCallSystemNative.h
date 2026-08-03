#pragma once

#include "RED4ext/RTTISystem.hpp"
#include "RED4ext/Scripting/IScriptable.hpp"
#include "RED4ext/Scripting/Stack.hpp"
#include "RED4ext/Scripting/Natives/Quaternion.hpp"
#include "RED4ext/Scripting/Natives/Vector4.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/VehicleSystem.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/data/VehicleType.hpp"

namespace RealisticVehicleCallSystem
{

class RealisticVehicleCallSystemNative : RED4ext::IScriptable
{
public:
    static void Hook();
    static char __fastcall hkFindSpawnLocation(RED4ext::gameVehicleSystem* vehicleSystem, float* playerPosition, float* outPosition, float* playerAndOutRotation);
    static bool __fastcall hkSpawnPlayerVehicle(RED4ext::gameVehicleSystem* vehicleSystem, RED4ext::gamedataVehicleType vehicleType, RED4ext::TweakDBID vehicleID, bool spawnOnlyOnValidRoad);
    static RED4ext::TweakDBID* __fastcall hkTweakDBIdCtorDerive(RED4ext::TweakDBID* base, RED4ext::TweakDBID* id, const char* name);
    static void TransactMoney(int quantity);

    static void SetSpawnPoint(RED4ext::IScriptable *aContext, RED4ext::CStackFrame *aFrame, RED4ext::CString *aOut, int64_t a4);
    static void ShowSimpleScreenMessage(RED4ext::IScriptable *aContext, RED4ext::CStackFrame *aFrame, RED4ext::CString *aOut, int64_t a4);

    RED4ext::CClass* GetNativeType();
};

constexpr const char* ToString(RED4ext::game::data::VehicleType type)
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
