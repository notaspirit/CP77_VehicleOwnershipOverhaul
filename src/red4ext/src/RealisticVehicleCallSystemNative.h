#pragma once

#include "RED4ext/RTTISystem.hpp"
#include "RED4ext/Scripting/IScriptable.hpp"
#include "RED4ext/Scripting/Stack.hpp"
#include "RED4ext/Scripting/Natives/Quaternion.hpp"
#include "RED4ext/Scripting/Natives/Vector4.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/VehicleSystem.hpp"

namespace RealisticVehicleCallSystem
{

class RealisticVehicleCallSystemNative : RED4ext::IScriptable
{
public:
    static void Hook();
    static char __fastcall hkFindSpawnLocation(RED4ext::gameVehicleSystem* vehicleSystem, float* playerPosition, float* outPosition, float* playerAndOutRotation);

    static void SetSpawnPoint(RED4ext::IScriptable *aContext, RED4ext::CStackFrame *aFrame, RED4ext::CString *aOut, int64_t a4);

    RED4ext::CClass* GetNativeType();
};

}
