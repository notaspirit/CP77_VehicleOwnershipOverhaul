#include <RealisticVehicleCallSystemNative.h>

#include <RedLib.hpp>

#include "GarageLoader.h"
#include "RedLogger.h"
#include "../vendor/MinHook/include/MinHook.h"
#include "DataStructs/Addresses.h"
#include "DataStructs/Garage.h"
#include "DataStructs/Globals.h"
#include "RED4ext/Scripting/Natives/Generated/game/SimpleScreenMessage.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/TransactionSystem.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/VehicleSystem.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/bb/AllScriptDefinitions.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/data/VehicleType.hpp"
#include "RED4ext/Scripting/Natives/Generated/physics/GeometryCache.hpp"

namespace RealisticVehicleCallSystem
{
std::vector<RealisticVehicleSystem::Garage> Garages;

RED4ext::gamedataVehicleType currentVehicleType;
RED4ext::TweakDBID currentVehicleID;
bool spawnedCurrentVehicle = false;
std::string lastDeliveredGarageName;

std::unordered_map<std::string, RED4ext::TweakDBID> RecordHashFlatNameIDMap;

RED4ext::Vector4 spawnPosition = RED4ext::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
RED4ext::Quaternion spawnRotation = RED4ext::Quaternion(0.0f, 0.0f, 0.0f, 1.0f);

typedef char (__fastcall* FindSpawnLocation_t)(RED4ext::gameVehicleSystem*, RED4ext::Vector3*, RED4ext::Vector3*, RED4ext::Quaternion*);
FindSpawnLocation_t oFindSpawnLocation = nullptr;

typedef bool (__fastcall* SpawnPlayerVehicle_t)(RED4ext::gameVehicleSystem*, RED4ext::gamedataVehicleType, RED4ext::TweakDBID, bool);
SpawnPlayerVehicle_t oSpawnPlayerVehicle = nullptr;

typedef RED4ext::TweakDBID* (__fastcall* TweakDBIdCtorDerive_t)(RED4ext::TweakDBID*, RED4ext::TweakDBID*, const char*);
TweakDBIdCtorDerive_t oTweakDBIdCtorDerive = nullptr;

typedef uint64_t (__fastcall* SummonVehicle_t)(long long param_1, uint32_t *param_2, uint64_t param_3, uint64_t param_4, uint64_t param_5, char param_6, uint8_t  param_7, uint32_t param_8, uint8_t  param_9, void*    param_10);
SummonVehicle_t oSummonVehicle = nullptr;

void RealisticVehicleCallSystemNative::Hook()
{
    RedLogger::Info("Hooking VehicleSystem");

    MH_Initialize();
    /*
    uintptr_t base = (uintptr_t)GetModuleHandleA(nullptr);

    void* findSpawnLocationAddress = (void*)(base + 0x25e64fc);
    void* spawnPlayerVehicleAddress = (void*)(base + 0x1ccec50);
    void* summonVehicleAddress = (void*)(base + 0x1ce7f80);
    */

    const RED4ext::UniversalRelocPtr<uint8_t> TweakDBIdConstructorMethod(Addresses::CScript_TDBIDConstructorDerive);
    uint8_t* TweakDBIdConstructorAddress = TweakDBIdConstructorMethod.GetAddr();

    const RED4ext::UniversalRelocPtr<uint8_t> FindSpawnLocationMethod(Addresses::gameVehicleSystem_FindSpawnLocation);
    uint8_t* findSpawnLocationAddress = FindSpawnLocationMethod.GetAddr();

    const RED4ext::UniversalRelocPtr<uint8_t> SpawnPlayerVehicleMethod(Addresses::gameVehicleSystem_SpawnPlayerVehicle);
    uint8_t* spawnPlayerVehicleAddress = SpawnPlayerVehicleMethod.GetAddr();

    const RED4ext::UniversalRelocPtr<uint8_t> SummonVehicleAddressMethod(Addresses::gameVehicleSystem_SummonVehicle);
    uint8_t* summonVehicleAddress = SummonVehicleAddressMethod.GetAddr();

    MH_CreateHook(findSpawnLocationAddress, &RealisticVehicleCallSystemNative::hkFindSpawnLocation, reinterpret_cast<void**>(&oFindSpawnLocation));
    MH_EnableHook(findSpawnLocationAddress);

    MH_CreateHook(spawnPlayerVehicleAddress, &RealisticVehicleCallSystemNative::hkSpawnPlayerVehicle, reinterpret_cast<void**>(&oSpawnPlayerVehicle));
    MH_EnableHook(spawnPlayerVehicleAddress);

    MH_CreateHook(TweakDBIdConstructorAddress, &RealisticVehicleCallSystemNative::hkTweakDBIdCtorDerive, reinterpret_cast<void**>(&oTweakDBIdCtorDerive));
    MH_EnableHook(TweakDBIdConstructorAddress);

    MH_CreateHook(summonVehicleAddress, &RealisticVehicleCallSystemNative::hkSummonVehicle, reinterpret_cast<void**>(&oSummonVehicle));
    MH_EnableHook(summonVehicleAddress);


    RedLogger::Info("Finished Hooking");
}
char __fastcall RealisticVehicleCallSystemNative::hkFindSpawnLocation(RED4ext::gameVehicleSystem* vehicleSystem, RED4ext::Vector3* playerPosition, RED4ext::Vector3* outPosition, RED4ext::Quaternion* playerAndOutRotation)
{
    char oResult = oFindSpawnLocation(vehicleSystem, playerPosition, outPosition, playerAndOutRotation);

    RedLogger::Info("Finished calling original findSpawnLocation method, setup variables for own method");

    if (!FindDeliveryPosition(playerPosition, currentVehicleType, currentVehicleID, outPosition, playerAndOutRotation))
    {
        RedLogger::Info("Finished FindDeliveryPosition with result false");
        return oResult;
    }

    RedLogger::Info("Finished FindDeliveryPosition with result true");

    return 1;
}

bool RealisticVehicleCallSystemNative::hkSpawnPlayerVehicle(RED4ext::gameVehicleSystem *vehicleSystem,
    RED4ext::gamedataVehicleType vehicleType, RED4ext::TweakDBID vehicleID, bool spawnOnlyOnValidRoad) {
    currentVehicleType = vehicleType;
    currentVehicleID = vehicleID;
    spawnedCurrentVehicle = false;

    bool result = oSpawnPlayerVehicle(vehicleSystem, vehicleType, vehicleID, spawnOnlyOnValidRoad);

    RedLogger::Info("[SpawnPlayerVehicle] result={}\nvehicleType: {}\nvehicleID: {}\nspawnOnlyOnValidRoad: {}", result, ToString(vehicleType), vehicleID.name.hash, spawnOnlyOnValidRoad);

    auto tdb = RED4ext::TweakDB::Get();

    auto displayNameFlatName =  std::to_string(vehicleID.name.hash) + ".displayName";
    auto displayNameFlatId = RecordHashFlatNameIDMap.find(displayNameFlatName);

    if (displayNameFlatId == RecordHashFlatNameIDMap.end())
    {
        RedLogger::Warning("Could not find flat for {}", displayNameFlatName);
        return result;
    }

    auto displayNameFlat = tdb->GetFlatValue(displayNameFlatId->second);
    auto locKey = displayNameFlat->GetValue<RED4ext::gamedataLocKeyWrapper>();
    RED4ext::CString locText;
    RED4ext::ExecuteGlobalFunction("GetLocalizedTextByKey", &locText, locKey->primaryKey);

    if (spawnedCurrentVehicle)
    {
        TransactMoney(-1000);

        RED4ext::CString logMessage = "Vehicle Delivery";
        RED4ext::ExecuteFunction("gameActivityLogSystem", "AddLog", nullptr, logMessage);


        RED4ext::SimpleScreenMessage message = { };
        message.isShown = true;
        message.duration = 5.0f;
        message.isInstant = true;
        message.message = std::format("Delivered {} to {}", locText.c_str(), lastDeliveredGarageName);
        message.type = RED4ext::SimpleMessageType::Neutral;

        RED4ext::ExecuteFunction("RealisticVehicleCallSystemNative", "ShowSimpleScreenMessage", nullptr, message);
    }
    else
    {
        RED4ext::SimpleScreenMessage message = { };
        message.isShown = true;
        message.duration = 5.0f;
        message.isInstant = true;
        message.message = std::string(locText.c_str()) + " is nearby";
        message.type = RED4ext::SimpleMessageType::Neutral;

        RED4ext::ExecuteFunction("RealisticVehicleCallSystemNative", "ShowSimpleScreenMessage", nullptr, message);
    }

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

uint64_t RealisticVehicleCallSystemNative::hkSummonVehicle(long long param_1, uint32_t *param_2, uint64_t param_3,
                                                            uint64_t param_4, uint64_t param_5, char param_6, uint8_t  param_7, uint32_t param_8, uint8_t  param_9, void*    param_10)
{
    spawnedCurrentVehicle = true;
    return oSummonVehicle(param_1, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10);
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

float Distance(const RED4ext::Vector3& a, const RED4ext::Vector3& b)
{
    float dx = a.X - b.X;
    float dy = a.Y - b.Y;
    float dz = a.Z - b.Z;

    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool GarageSupports(const RealisticVehicleSystem::Garage& garage, const RED4ext::gamedataVehicleType vehicleType, const RED4ext::TweakDBID vehicleId)
{
    if (!garage.VehicleTypes.at(vehicleType))
        return false;

    if (!garage.WhiteListedVehicles.empty() && !garage.WhiteListedVehicles.contains(vehicleId))
        return false;

    if (!garage.BlackListedVehicles.empty() && garage.BlackListedVehicles.contains(vehicleId))
        return false;

    return true;
}

bool SlotSupports(const RealisticVehicleSystem::Slot& slot, const RED4ext::gamedataVehicleType vehicleType, const RED4ext::TweakDBID vehicleId)
{
    if (!slot.VehicleTypes.at(vehicleType))
        return false;

    if (!slot.WhiteListedVehicles.empty() && !slot.WhiteListedVehicles.contains(vehicleId))
        return false;

    if (!slot.BlackListedVehicles.empty() && slot.BlackListedVehicles.contains(vehicleId))
        return false;

    return true;
}

bool IsGarageBought(RealisticVehicleSystem::Garage garage)
{
    if (garage.QuestFact.empty())
        return true;

    int32_t result;
    RED4ext::CString questFactCString(garage.QuestFact.c_str());
    RED4ext::ExecuteFunction("questQuestsSystem", "GetFactStr", &result, questFactCString);

    RedLogger::Info("checked quest fact using questQuestsSystem.GetFactStr() with CString: {} (original string: {}) result is {}", questFactCString.c_str(), garage.QuestFact, result);

    return result == 1;
}

bool RealisticVehicleCallSystemNative::FindDeliveryPosition(RED4ext::Vector3* playerPosition,
    const RED4ext::gamedataVehicleType vehicleType,
    const RED4ext::TweakDBID vehicleId,
    RED4ext::Vector3* outPosition,
    RED4ext::Quaternion* outRotation)
{
    RedLogger::Info("Finding delivery position for vehicle type {} and id {}", ToString(vehicleType), vehicleId.name.hash);
    RedLogger::Info("{} garages available" , Garages.size());
    if (Garages.empty())
        return false;

    RealisticVehicleSystem::Garage closestGarage;
    float closestDistance = std::numeric_limits<float>::max();
    bool found = false;

    for (const auto& garage : Garages)
    {
        RedLogger::Info("Checking garage {}" , garage.Name);

        if (!IsGarageBought(garage))
        {
            RedLogger::Info("Garage {} is not bought", garage.Name);
            continue;
        }

        if (!GarageSupports(garage, vehicleType, vehicleId))
        {
            RedLogger::Info("Garage {} does not support vehicle type {} and id {}", garage.Name, ToString(vehicleType), vehicleId.name.hash);
            continue;
        }

        found = true;

        auto distance = Distance(*playerPosition, garage.Position);
        RedLogger::Info("Distance to garage {} is {}", garage.Name, distance);
        if (distance < closestDistance)
        {
            RedLogger::Info("New closest garage {} with distance {}", garage.Name, distance);

            closestGarage = garage;
            closestDistance = distance;
        }
    }

    std::vector<RealisticVehicleSystem::Slot> matchingSlots;

    for (const auto& slot : closestGarage.Slots)
        if (SlotSupports(slot, vehicleType, vehicleId))
            matchingSlots.push_back(slot);

    RedLogger::Info("{} matching slots found", matchingSlots.size());

    if (matchingSlots.empty())
        return false;

    auto randomSlot = g_rng.getInt32(matchingSlots.size() - 1, 0);

    RedLogger::Info("Random slot is {}", randomSlot);

    const auto& selectedSlot = matchingSlots.at(randomSlot);

    outPosition->X = selectedSlot.Position.X;
    outPosition->Y = selectedSlot.Position.Y;
    outPosition->Z = selectedSlot.Position.Z;

    outRotation->i = selectedSlot.Rotation.i;
    outRotation->j = selectedSlot.Rotation.j;
    outRotation->k = selectedSlot.Rotation.k;
    outRotation->r = selectedSlot.Rotation.r;

    lastDeliveredGarageName = closestGarage.Name;

    return found;
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

void RealisticVehicleCallSystemNative::ShowSimpleScreenMessage(
    RED4ext::IScriptable *aContext,
    RED4ext::CStackFrame *aFrame,
    RED4ext::CString *aOut,
    int64_t a4)
{
    RED4ext::SimpleScreenMessage message {};
    RED4ext::GetParameter(aFrame, &message);
    aFrame->code++;

    // do nothing, CET script will observe method call and post message
}

void RealisticVehicleCallSystemNative::sInitialize(
    RED4ext::IScriptable *aContext,
    RED4ext::CStackFrame *aFrame,
    RED4ext::CString *aOut,
    int64_t a4)
{
    aFrame->code++;

    Initialize();
}

void RealisticVehicleCallSystemNative::Initialize()
{
    g_rng.state = std::chrono::steady_clock::now().time_since_epoch().count();
    Garages = GarageLoader::LoadGarages();
}
}

