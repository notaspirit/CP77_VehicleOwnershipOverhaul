#pragma once

#include "RED4ext/RTTISystem.hpp"
#include "RED4ext/Scripting/IScriptable.hpp"
#include "RED4ext/Scripting/Stack.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/VehicleSystem.hpp"

namespace RealisticVehicleCallSystem
{

class RealisticVehicleCallSystemNative : RED4ext::IScriptable
{
public:
    static void Hook();
    static char __fastcall hkFindSpawnLocation(RED4ext::gameVehicleSystem* vehicleSystem, float* playerPosition, float* outPosition, float* playerAndOutRotation);

    RED4ext::CClass* GetNativeType();
};

}
