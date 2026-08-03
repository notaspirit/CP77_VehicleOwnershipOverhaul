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

    float myX = playerPosition[0], myY = playerPosition[1], myZ = playerPosition[2] + 20.0f;
    outPosition[0] = myX; outPosition[1] = myY; outPosition[2] = myZ;

    playerAndOutRotation[0] = 0.0f;
    playerAndOutRotation[1] = 0.0f;
    playerAndOutRotation[2] = 0.7071f;
    playerAndOutRotation[3] = 0.7071f;

    return 1;
}
}

