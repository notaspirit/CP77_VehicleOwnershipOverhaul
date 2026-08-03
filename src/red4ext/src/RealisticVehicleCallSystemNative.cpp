#include <RealisticVehicleCallSystemNative.h>

#include <RedLib.hpp>
#include "RedLogger.h"
#include "../vendor/MinHook/include/MinHook.h"
#include "DataStructs/Addresses.h"
#include "DataStructs/Globals.h"
#include "RED4ext/Scripting/Natives/Generated/game/TransactionSystem.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/VehicleSystem.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/data/VehicleType.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/data/Vehicle_Record.hpp"
#include "RED4ext/Scripting/Natives/Generated/physics/GeometryCache.hpp"

namespace RealisticVehicleCallSystem
{

std::unordered_map<std::string, RED4ext::TweakDBID> RecordHashFlatNameIDMap;

RED4ext::Vector4 spawnPosition = RED4ext::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
RED4ext::Quaternion spawnRotation = RED4ext::Quaternion(0.0f, 0.0f, 0.0f, 1.0f);

typedef char (__fastcall* FindSpawnLocation_t)(RED4ext::gameVehicleSystem*, float*, float*, float*);
FindSpawnLocation_t oFindSpawnLocation = nullptr;

typedef bool (__fastcall* SpawnPlayerVehicle_t)(RED4ext::gameVehicleSystem*, RED4ext::gamedataVehicleType, RED4ext::TweakDBID, bool);
SpawnPlayerVehicle_t oSpawnPlayerVehicle = nullptr;

typedef RED4ext::TweakDBID* (__fastcall* TweakDBIdCtorDerive_t)(RED4ext::TweakDBID*, RED4ext::TweakDBID*, const char*);
TweakDBIdCtorDerive_t oTweakDBIdCtorDerive = nullptr;

void RealisticVehicleCallSystemNative::Hook()
{
    RedLogger::Info("Hooking VehicleSystem");

    MH_Initialize();

    uintptr_t base = (uintptr_t)GetModuleHandleA(nullptr);
    void* findSpawnLocationAddress = (void*)(base + 0x25e64fc);
    void* spawnPlayerVehicleAddress = (void*)(base + 0x1ccec50);

    const RED4ext::UniversalRelocPtr<uint8_t> TweakDBIdConstructorMethod(Addresses::CScript_TDBIDConstructorDerive);
    uint8_t* TweakDBIdConstructorAddress = TweakDBIdConstructorMethod.GetAddr();

    MH_CreateHook(findSpawnLocationAddress, &RealisticVehicleCallSystemNative::hkFindSpawnLocation, reinterpret_cast<void**>(&oFindSpawnLocation));
    MH_EnableHook(findSpawnLocationAddress);

    MH_CreateHook(spawnPlayerVehicleAddress, &RealisticVehicleCallSystemNative::hkSpawnPlayerVehicle, reinterpret_cast<void**>(&oSpawnPlayerVehicle));
    MH_EnableHook(spawnPlayerVehicleAddress);

    MH_CreateHook(TweakDBIdConstructorAddress, &RealisticVehicleCallSystemNative::hkTweakDBIdCtorDerive, reinterpret_cast<void**>(&oTweakDBIdCtorDerive));
    MH_EnableHook(TweakDBIdConstructorAddress);


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

    auto tdb = RED4ext::TweakDB::Get();

    auto displayNameFlatName =  std::to_string(vehicleID.name.hash) + ".displayName";
    auto displayNameFlatId = RecordHashFlatNameIDMap.find(displayNameFlatName);

    if (displayNameFlatId == RecordHashFlatNameIDMap.end())
        RedLogger::Warning("Could not find flat for " + displayNameFlatName);
    else
    {
        auto displayNameFlat = tdb->GetFlatValue(displayNameFlatId->second);
        auto locKey = displayNameFlat->GetValue<RED4ext::gamedataLocKeyWrapper>();

        RedLogger::Info("LocKey for display name is: " + std::to_string(locKey->primaryKey));

        RED4ext::CString locText;
        RED4ext::ExecuteGlobalFunction("GetLocalizedTextByKey", &locText, locKey->primaryKey);

        RedLogger::Info("Display name is: " + std::string(locText.c_str()));
    }

    TransactMoney(-1000);

    return result;
}

RED4ext::TweakDBID* RealisticVehicleCallSystemNative::hkTweakDBIdCtorDerive(RED4ext::TweakDBID* base,
    RED4ext::TweakDBID* id, const char* name)
{
    auto result = oTweakDBIdCtorDerive(base, id, name);
    auto stringId = std::to_string(base->name.hash) + std::string(name);
    RecordHashFlatNameIDMap[stringId] = *id;
    return result;
}

// based on CETs AddItemToInventory under MIT
void RealisticVehicleCallSystemNative::TransactMoney(int quantity)
{
    RED4ext::ScriptGameInstance gameInstance;
    RED4ext::Handle<RED4ext::IScriptable> player;
    RED4ext::ExecuteGlobalFunction("GetPlayer;GameInstance", &player, gameInstance);

    bool result;
    RED4ext::TweakDBID itemID("Items.money");
    RED4ext::ExecuteFunction("gameTransactionSystem", "GiveItemByTDBID", &result, player, itemID, quantity);
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

