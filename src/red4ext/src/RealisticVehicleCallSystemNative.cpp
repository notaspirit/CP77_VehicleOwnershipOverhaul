#include <RealisticVehicleCallSystemNative.h>

#include <RedLib.hpp>
#include "RedLogger.h"
#include "../vendor/MinHook/include/MinHook.h"
#include "DataStructs/Addresses.h"
#include "DataStructs/Globals.h"
#include "RED4ext/Scripting/Natives/Generated/physics/GeometryCache.hpp"

namespace RealisticVehicleCallSystem
{
typedef char (__fastcall* FindSpawnLocation_t)(int64_t, float*, float*, void*);
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
char __fastcall RealisticVehicleCallSystemNative::hkFindSpawnLocation(int64_t param_1, float* param_2, float* param_3, void* param_4)
{
    oFindSpawnLocation(param_1, param_2, param_3, param_4);

    float myX = param_2[0], myY = param_2[1], myZ = param_2[2] + 20.0f;
    param_3[0] = myX; param_3[1] = myY; param_3[2] = myZ;

    float* rot = (float*)param_4;
    rot[0] = 0.0f;
    rot[1] = 0.0f;
    rot[2] = 0.7071f;
    rot[3] = 0.7071f;

    return 1;
}
}

