#pragma once

#include "RED4ext/RTTISystem.hpp"
#include "RED4ext/Scripting/IScriptable.hpp"
#include "RED4ext/Scripting/Stack.hpp"

namespace RealisticVehicleCallSystem
{

class RealisticVehicleCallSystemNative : RED4ext::IScriptable
{
public:
    static void Hook();
    static char __fastcall hkFindSpawnLocation(int64_t param_1, float* param_2, float* param_3, void* param_4);

    RED4ext::CClass* GetNativeType();
};

}
