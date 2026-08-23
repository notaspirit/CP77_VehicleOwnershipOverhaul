#include <RealisticVehicleCallSystemNative.h>

#include <RedLib.hpp>

#include "RedLogger.h"
#include "../vendor/MinHook/include/MinHook.h"
#include "DataStructs/Addresses.h"
#include "DataStructs/Globals.h"
#include "RED4ext/Scripting/Natives/physicsTraceResult.hpp"
#include "RED4ext/Scripting/Natives/Generated/WorldTransform.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/SimpleScreenMessage.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/VehicleSystem.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/data/VehicleType.hpp"

namespace RealisticVehicleCallSystem
{
typedef char (__fastcall* FindSpawnLocation_t)(RED4ext::gameVehicleSystem*, RED4ext::Vector3*, RED4ext::Vector3*, RED4ext::Quaternion*);
FindSpawnLocation_t oFindSpawnLocation = nullptr;

typedef bool (__fastcall* SpawnPlayerVehicle_t)(RED4ext::gameVehicleSystem*, RED4ext::gamedataVehicleType, RED4ext::TweakDBID, bool);
SpawnPlayerVehicle_t oSpawnPlayerVehicle = nullptr;

typedef uint64_t (__fastcall* SummonVehicle_t)(long long param_1, uint32_t *param_2, uint64_t param_3, uint64_t param_4, uint64_t param_5, char param_6, uint8_t  param_7, uint32_t param_8, uint8_t  param_9, void*    param_10);
SummonVehicle_t oSummonVehicle = nullptr;

void RealisticVehicleCallSystemNative::Hook()
{
    RedLogger::Info("Hooking VehicleSystem");

    MH_Initialize();

    const RED4ext::UniversalRelocPtr<uint8_t> FindSpawnLocationMethod(Addresses::gameVehicleSystem_FindSpawnLocation);
    uint8_t* findSpawnLocationAddress = FindSpawnLocationMethod.GetAddr();

    const RED4ext::UniversalRelocPtr<uint8_t> SpawnPlayerVehicleMethod(Addresses::gameVehicleSystem_SpawnPlayerVehicle);
    uint8_t* spawnPlayerVehicleAddress = SpawnPlayerVehicleMethod.GetAddr();

    const RED4ext::UniversalRelocPtr<uint8_t> SummonVehicleAddressMethod(Addresses::gameVehicleSystem_SummonVehicle);
    uint8_t* summonVehicleAddress = SummonVehicleAddressMethod.GetAddr();

    auto createHkFindSpawnLocation = MH_CreateHook(findSpawnLocationAddress, &RealisticVehicleCallSystemNative::hkFindSpawnLocation, reinterpret_cast<void**>(&oFindSpawnLocation));
    auto enableHkFindSpawnLocation = MH_EnableHook(findSpawnLocationAddress);

    if (createHkFindSpawnLocation == MH_OK && enableHkFindSpawnLocation == MH_OK)
        RedLogger::Info("Hooked gameVehicleSystem_FindSpawnLocation");
    else
        RedLogger::Error("Failed to hook gameVehicleSystem_FindSpawnLocation. Create: {} Enable: {}", MH_StatusToString(createHkFindSpawnLocation), MH_StatusToString(enableHkFindSpawnLocation));

    auto createHkSpawnPlayerVehicle = MH_CreateHook(spawnPlayerVehicleAddress, &RealisticVehicleCallSystemNative::hkSpawnPlayerVehicle, reinterpret_cast<void**>(&oSpawnPlayerVehicle));
    auto enableHkSpawnPlayerVehicle = MH_EnableHook(spawnPlayerVehicleAddress);

    if (createHkSpawnPlayerVehicle == MH_OK && enableHkSpawnPlayerVehicle == MH_OK)
        RedLogger::Info("Hooked gameVehicleSystem_SpawnPlayerVehicle");
    else
        RedLogger::Error("Failed to hook gameVehicleSystem_SpawnPlayerVehicle. Create: {} Enable: {}", MH_StatusToString(createHkSpawnPlayerVehicle), MH_StatusToString(enableHkSpawnPlayerVehicle));

    auto createHkSummonVehicle = MH_CreateHook(summonVehicleAddress, &RealisticVehicleCallSystemNative::hkSummonVehicle, reinterpret_cast<void**>(&oSummonVehicle));
    auto enableHkSummonVehicle = MH_EnableHook(summonVehicleAddress);

    if (createHkSummonVehicle == MH_OK && enableHkSummonVehicle == MH_OK)
        RedLogger::Info("Hooked gameVehicleSystem_SummonVehicle");
    else
        RedLogger::Error("Failed to hook gameVehicleSystem_SummonVehicle. Create: {} Enable: {}", MH_StatusToString(createHkSummonVehicle), MH_StatusToString(enableHkSummonVehicle));

    RedLogger::Info("Finished Hooking");
}
char __fastcall RealisticVehicleCallSystemNative::hkFindSpawnLocation(RED4ext::gameVehicleSystem* vehicleSystem, RED4ext::Vector3* playerPosition, RED4ext::Vector3* outPosition, RED4ext::Quaternion* playerAndOutRotation)
{
    RED4ext::WorldTransform outTransform;
    RED4ext::ExecuteFunction("RealisticVehicleCallSystemNative", "hkPreFindSpawnLocation", &outTransform, playerPosition, outPosition, playerAndOutRotation);

    auto pos = outTransform.Position.AsVector3();
    auto rot = outTransform.Orientation;

    if (pos.X == 0 && pos.Y == 0 && pos.Z == 0 &&
        rot.i == 0 && rot.j == 0 && rot.k == 0 && rot.r == 1)
        return oFindSpawnLocation(vehicleSystem, playerPosition, outPosition, playerAndOutRotation);

    outPosition->X = pos.X;
    outPosition->Y = pos.Y;
    outPosition->Z = pos.Z;

    playerAndOutRotation->i = rot.i;
    playerAndOutRotation->j = rot.j;
    playerAndOutRotation->k = rot.k;
    playerAndOutRotation->r = rot.r;

    return 1;
}

bool RealisticVehicleCallSystemNative::hkSpawnPlayerVehicle(RED4ext::gameVehicleSystem *vehicleSystem,
    RED4ext::gamedataVehicleType vehicleType, RED4ext::TweakDBID vehicleID, bool spawnOnlyOnValidRoad)
{
    RED4ext::ExecuteFunction("RealisticVehicleCallSystemNative", "hkPreSpawnPlayerVehicle", nullptr, vehicleType, vehicleID, spawnOnlyOnValidRoad);

    bool result = oSpawnPlayerVehicle(vehicleSystem, vehicleType, vehicleID, spawnOnlyOnValidRoad);

    RED4ext::ExecuteFunction("RealisticVehicleCallSystemNative", "hkPostSpawnPlayerVehicle", nullptr, vehicleType, vehicleID, spawnOnlyOnValidRoad);

    return result;
}

uint64_t RealisticVehicleCallSystemNative::hkSummonVehicle(long long param_1, uint32_t *param_2, uint64_t param_3,
                                                            uint64_t param_4, uint64_t param_5, char param_6, uint8_t  param_7, uint32_t param_8, uint8_t  param_9, void*    param_10)
{
    RED4ext::ExecuteFunction("RealisticVehicleCallSystemNative", "hkPreSummonVehicle", nullptr);

    return oSummonVehicle(param_1, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10);
}

void RealisticVehicleCallSystemNative::hkPreSpawnPlayerVehicleRTTI(RED4ext::IScriptable *aContext,
    RED4ext::CStackFrame *aFrame, RED4ext::CString *aOut, int64_t a4)
{
    RED4ext::gamedataVehicleType vehicleType;
    RED4ext::TweakDBID vehicleID;
    bool spawnOnlyOnValidRoad;

    RED4ext::GetParameter(aFrame, &vehicleType);
    RED4ext::GetParameter(aFrame, &vehicleID);
    RED4ext::GetParameter(aFrame, &spawnOnlyOnValidRoad);

    aFrame->code++;
}

void RealisticVehicleCallSystemNative::hkPreFindSpawnLocationRTTI(RED4ext::IScriptable *aContext,
    RED4ext::CStackFrame *aFrame, RED4ext::WorldTransform *aOut, int64_t a4)
{
    RED4ext::Vector3 playerPosition;
    RED4ext::Vector3 outPosition;
    RED4ext::Quaternion playerAndOutRotation;

    RED4ext::GetParameter(aFrame, &playerPosition);
    RED4ext::GetParameter(aFrame, &outPosition);
    RED4ext::GetParameter(aFrame, &playerAndOutRotation);

    aFrame->code++;

    if (aOut)
    {
        *aOut = RED4ext::WorldTransform();
    }
}

void RealisticVehicleCallSystemNative::hkPreSummonVehicleRTTI(RED4ext::IScriptable *aContext, RED4ext::CStackFrame *aFrame,
    RED4ext::CString *aOut, int64_t a4)
{
    aFrame->code++;
}

void RealisticVehicleCallSystemNative::hkPostSpawnPlayerVehicleRTTI(RED4ext::IScriptable *aContext,
    RED4ext::CStackFrame *aFrame, RED4ext::CString *aOut, int64_t a4)
{
    RED4ext::gamedataVehicleType vehicleType;
    RED4ext::TweakDBID vehicleID;
    bool spawnOnlyOnValidRoad;

    RED4ext::GetParameter(aFrame, &vehicleType);
    RED4ext::GetParameter(aFrame, &vehicleID);
    RED4ext::GetParameter(aFrame, &spawnOnlyOnValidRoad);

    aFrame->code++;
}
}

