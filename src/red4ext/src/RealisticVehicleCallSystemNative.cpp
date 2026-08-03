#include <RealisticVehicleCallSystemNative.h>

#include <RedLib.hpp>
#include "RedLogger.h"
#include "../vendor/MinHook/include/MinHook.h"
#include "DataStructs/Addresses.h"
#include "DataStructs/Globals.h"
#include "RED4ext/Scripting/Natives/Generated/game/VehicleSystem.hpp"
#include "RED4ext/Scripting/Natives/Generated/physics/GeometryCache.hpp"

namespace RealisticVehicleCallSystem
{

RED4ext::Vector4 spawnPosition = RED4ext::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
RED4ext::Quaternion spawnRotation = RED4ext::Quaternion(0.0f, 0.0f, 0.0f, 1.0f);

typedef char (__fastcall* FindSpawnLocation_t)(RED4ext::gameVehicleSystem*, float*, float*, float*);
FindSpawnLocation_t oFindSpawnLocation = nullptr;

void RealisticVehicleCallSystemNative::Hook()
{
    RedLogger::Info("Hooking VehicleSystem");

    MH_Initialize();

    uintptr_t base = (uintptr_t)GetModuleHandleA(nullptr);
    void* target = (void*)(base + 0x25e64fc);

    MH_CreateHook(target, &RealisticVehicleCallSystemNative::hkFindSpawnLocation, reinterpret_cast<void**>(&oFindSpawnLocation));
    MH_EnableHook(target);

    RedLogger::Info("Finished Hooking");
}
char __fastcall RealisticVehicleCallSystemNative::hkFindSpawnLocation(RED4ext::gameVehicleSystem* vehicleSystem, float* playerPosition, float* outPosition, float* playerAndOutRotation)
{
    oFindSpawnLocation(vehicleSystem, playerPosition, outPosition, playerAndOutRotation);

    outPosition[0] = spawnPosition.X;
    outPosition[1] = spawnPosition.Y;
    outPosition[2] = spawnPosition.Z;

    playerAndOutRotation[0] = spawnRotation.i;
    playerAndOutRotation[1] = spawnRotation.j;
    playerAndOutRotation[2] = spawnRotation.k;
    playerAndOutRotation[3] = spawnRotation.r;

    return 1;
}

void RealisticVehicleCallSystemNative::SetSpawnPoint(
    RED4ext::IScriptable *aContext,
    RED4ext::CStackFrame *aFrame,
    RED4ext::CString *aOut,
    int64_t a4)
{
    RED4ext::Vector4 position {};
    RED4ext::Quaternion rotation {};
    RED4ext::GetParameter(aFrame, &position);
    RED4ext::GetParameter(aFrame, &rotation);
    aFrame->code++;

    spawnPosition = position;
    spawnRotation = rotation;
}
}

