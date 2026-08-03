#include <RealisticVehicleCallSystemNative.h>

#include <RedLib.hpp>
#include "RedLogger.h"
#include "../vendor/MinHook/include/MinHook.h"
#include "DataStructs/Addresses.h"
#include "DataStructs/Globals.h"
#include "RED4ext/Scripting/Natives/Generated/game/VehicleSystem.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/data/VehicleType.hpp"
#include "RED4ext/Scripting/Natives/Generated/physics/GeometryCache.hpp"

namespace RealisticVehicleCallSystem
{

RED4ext::Vector4 spawnPosition = RED4ext::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
RED4ext::Quaternion spawnRotation = RED4ext::Quaternion(0.0f, 0.0f, 0.0f, 1.0f);

typedef char (__fastcall* FindSpawnLocation_t)(RED4ext::gameVehicleSystem*, float*, float*, float*);
FindSpawnLocation_t oFindSpawnLocation = nullptr;

typedef bool (__fastcall* SpawnPlayerVehicle_t)(RED4ext::gameVehicleSystem*, RED4ext::gamedataVehicleType, RED4ext::TweakDBID, bool);
SpawnPlayerVehicle_t oSpawnPlayerVehicle = nullptr;

void RealisticVehicleCallSystemNative::Hook()
{
    RedLogger::Info("Hooking VehicleSystem");

    MH_Initialize();

    uintptr_t base = (uintptr_t)GetModuleHandleA(nullptr);
    void* findSpawnLocationAddress = (void*)(base + 0x25e64fc);
    void* spawnPlayerVehicleAddress = (void*)(base + 0x1ccec50);

    MH_CreateHook(findSpawnLocationAddress, &RealisticVehicleCallSystemNative::hkFindSpawnLocation, reinterpret_cast<void**>(&oFindSpawnLocation));
    MH_EnableHook(findSpawnLocationAddress);

    MH_CreateHook(spawnPlayerVehicleAddress, &RealisticVehicleCallSystemNative::hkSpawnPlayerVehicle, reinterpret_cast<void**>(&oSpawnPlayerVehicle));
    MH_EnableHook(spawnPlayerVehicleAddress);

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

bool RealisticVehicleCallSystemNative::hkSpawnPlayerVehicle(RED4ext::gameVehicleSystem *vehicleSystem,
    RED4ext::gamedataVehicleType vehicleType, RED4ext::TweakDBID vehicleID, bool spawnOnlyOnValidRoad)
{
    bool result = oSpawnPlayerVehicle(vehicleSystem, vehicleType, vehicleID, spawnOnlyOnValidRoad);

    char buf[512];

    snprintf(buf, sizeof(buf),
        "[SpawnPlayerVehicle] result=%d\n"
        "  vehicleType: %s\n"
        "  vehicleID: %u\n"
        "  spawnOnlyOnValidRoad: %d",
        result,
        ToString(vehicleType),
        vehicleID.name.hash,
        spawnOnlyOnValidRoad
    );

    RedLogger::Info(buf);

    return result;
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

