#include "GarageLoader.h"

#include <format>
#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <RapidJson/document.h>
#include <RapidJson/error/en.h>

#include "GenericHelpers.h"
#include "RapidJsonHelper.h"
#include "RedLogger.h"
#include "DataStructs/Globals.h"
#include "RED4ext/Scripting/Natives/Generated/game/data/AIAbilityCond_Record.hpp"

namespace RealisticVehicleCallSystem
{
std::filesystem::path GetExeDir() {
    wchar_t buffer[MAX_PATH + 1];

    DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (len == ERROR_INSUFFICIENT_BUFFER)
        throw ERROR_INSUFFICIENT_BUFFER;

    return std::filesystem::path(buffer).parent_path();
}

std::tuple<int, size_t> GetTrailingNumber(const std::string& str)
{
    size_t end = str.size();
    size_t start = end;

    while (start > 0 && std::isdigit(static_cast<unsigned char>(str[start - 1])))
        --start;

    if (start == end)
        return {-1, 0}; // No trailing number

    return {std::stoi(str.substr(start)), start};
}

std::vector<RealisticVehicleSystem::Garage> GarageLoader::LoadGarages()
{
    std::vector<RealisticVehicleSystem::Garage> garages {};
    std::unordered_set<std::string> encounteredNames;

    RedLogger::Info("Loading garage files");

    std::string garagesDir;
    try {
        garagesDir = GetExeDir().string() + R"(\plugins\cyber_engine_tweaks\mods\###RealisticVehicleCallSystem\data\garages)";
    }
    catch (const std::exception& e) {
        RedLogger::Error("Failed to get executable directory. Cannot load garage files.");
        return {};
    }

    for (const auto& garageFile : std::filesystem::directory_iterator(garagesDir))
    {
        auto filePath = garageFile.path().string();
        auto fileName = garageFile.path().filename().string();
        if (!filePath.ends_with(".json") || !garageFile.is_regular_file())
        {
            continue;
        }

        rapidjson::Document doc;
        std::ifstream fileStream(filePath);
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        doc.Parse(buffer.str().c_str());

        RedLogger::Info("Loading garage file {}", fileName);

        if (doc.HasParseError()) {
            RedLogger::Error("Failed to parse category file with error {}.", rapidjson::GetParseError_En(doc.GetParseError()));
            continue;
        }

        if (!RapidJsonHelper::IsObjectArray(doc))
        {
            RedLogger::Error("Failed to load garage file {}: File must be array of garages.", fileName);
            continue;
        }

        auto i = 0;
        for (const auto& garageJson : doc.GetArray())
        {
            RealisticVehicleSystem::Garage garage;

            if (!RapidJsonHelper::TryGetStringValue(garageJson, "Name", garage.Name))
            {
                RedLogger::Error("Failed to load garage index {} in file {} : Garage must have a Name property of type string", i, fileName);
                i++;
                continue;
            }

            while (encounteredNames.contains(garage.Name))
            {
                auto [number, firstDigitIndex] = GetTrailingNumber(garage.Name);

                if (number == -1)
                    garage.Name = std::format("{}_1", garage.Name);
                else
                    garage.Name = std::format("{}{}", garage.Name.substr(0, firstDigitIndex), number + 1);
            }

            encounteredNames.insert(garage.Name);

            if (!RapidJsonHelper::TryGetStringValue(garageJson, "QuestFact", garage.QuestFact))
            {
                RedLogger::Error("Failed to load garage index {} in file {} : Garage must have a QuestFact property of type string", i, fileName);
                i++;
                continue;
            }


            if (!RapidJsonHelper::IsObjectArray(garageJson, "Slots"))
            {
                RedLogger::Error("Failed to load garage index {} in file {} : Garage must have a Slots property of type array of object", i, fileName);
                i++;
                continue;
            }

            std::vector<float> slotPositionsX;
            std::vector<float> slotPositionsY;
            std::vector<float> slotPositionsZ;

            garage.VehicleTypes = {};
            garage.VehicleTypes[RED4ext::gamedataVehicleType::Bike] = false;
            garage.VehicleTypes[RED4ext::gamedataVehicleType::Car] = false;
            garage.VehicleTypes[RED4ext::gamedataVehicleType::Panzer] = false;
            garage.VehicleTypes[RED4ext::gamedataVehicleType::Count] = false;
            garage.VehicleTypes[RED4ext::gamedataVehicleType::Invalid] = false;

            auto si = 0;
            for (const auto& slotJson : garageJson["Slots"].GetArray())
            {
                RealisticVehicleSystem::Slot slot;

                slot.VehicleTypes = {};
                slot.VehicleTypes[RED4ext::gamedataVehicleType::Bike] = false;
                slot.VehicleTypes[RED4ext::gamedataVehicleType::Car] = false;
                slot.VehicleTypes[RED4ext::gamedataVehicleType::Panzer] = false;
                slot.VehicleTypes[RED4ext::gamedataVehicleType::Count] = false;
                slot.VehicleTypes[RED4ext::gamedataVehicleType::Invalid] = false;

                if (!RapidJsonHelper::IsStringArray(slotJson, "VehicleTypes"))
                {
                    RedLogger::Error("Failed to load slot index {} garage index {} in file {} : Garage slot must have a VehicleTypes property of type array of strings", si, i, fileName);
                    si++;
                    continue;
                }

                for (const auto& vehicleTypeJson : slotJson["VehicleTypes"].GetArray())
                {
                    if (strcmp(vehicleTypeJson.GetString(), "Bike"))
                        slot.VehicleTypes[RED4ext::gamedataVehicleType::Bike] = true;
                    else if (strcmp(vehicleTypeJson.GetString(), "Car"))
                        slot.VehicleTypes[RED4ext::gamedataVehicleType::Car] = true;
                    else if (strcmp(vehicleTypeJson.GetString(), "Panzer"))
                        slot.VehicleTypes[RED4ext::gamedataVehicleType::Panzer] = true;
                    else
                        RedLogger::Warning("Unknown vehicle type {} in slot index {} garage index {} in file {}", vehicleTypeJson.GetString(), si, i, fileName);
                }

                if (!RapidJsonHelper::IsStringArray(slotJson, "WhiteListedVehicles"))
                {
                    RedLogger::Error("Failed to load slot index {} garage index {} in file {} : Garage slot must have a WhiteListedVehicles property of type array of strings", si, i, fileName);
                    si++;
                    continue;
                }

                for (const auto& whiteListedVehicleJson : slotJson["WhiteListedVehicles"].GetArray())
                    slot.WhiteListedVehicles.insert(whiteListedVehicleJson.GetString());

                if (!RapidJsonHelper::IsStringArray(slotJson, "BlackListedVehicles"))
                {
                    RedLogger::Error("Failed to load slot index {} garage index {} in file {} : Garage slot must have a BlackListedVehicles property of type array of strings", si, i, fileName);
                    si++;
                    continue;
                }

                for (const auto& blackListedVehicleJson : slotJson["BlackListedVehicles"].GetArray())
                    slot.BlackListedVehicles.insert(blackListedVehicleJson.GetString());


                if (!RapidJsonHelper::IsObject(slotJson, "Position"))
                {
                    RedLogger::Error("Failed to load slot index {} in garage index {} in file {} : Garage slot must have a Position property of type object", si, i, fileName);
                    si++;
                    continue;
                }

                float x, y, z;

                if (!RapidJsonHelper::TryGetFloatValue(slotJson["Position"], "X", x) ||
                    !RapidJsonHelper::TryGetFloatValue(slotJson["Position"], "Y", y) ||
                    !RapidJsonHelper::TryGetFloatValue(slotJson["Position"], "Z", z))
                {
                    RedLogger::Error("Failed to load slot index {} in garage index {} in file {} : Garage slot Position must have X, Y and Z properties of type float", si, i, fileName);
                    si++;
                    continue;
                }

                slot.Position = { x, y, z };

                if (!RapidJsonHelper::IsObject(slotJson, "Rotation"))
                {
                    RedLogger::Error("Failed to load slot index {} in garage index {} in file {} : Garage slot must have a Rotation property of type object", si, i, fileName);
                    si++;
                    continue;
                }

                float ri, rj, rk, rr;

                if (!RapidJsonHelper::TryGetFloatValue(slotJson["Rotation"], "I", ri) ||
                    !RapidJsonHelper::TryGetFloatValue(slotJson["Rotation"], "J", rj) ||
                    !RapidJsonHelper::TryGetFloatValue(slotJson["Rotation"], "K", rk) ||
                    !RapidJsonHelper::TryGetFloatValue(slotJson["Rotation"], "R", rr))
                {
                    RedLogger::Error("Failed to load slot index {} in garage index {} in file {} : Garage slot Rotation must have I, J, K and R properties of type float", si, i, fileName);
                    si++;
                    continue;
                }

                slot.Rotation = { ri, rj, rk, rr };

                slotPositionsX.push_back(x);
                slotPositionsY.push_back(y);
                slotPositionsZ.push_back(z);

                for (const auto& vehicleType : slot.VehicleTypes)
                    garage.VehicleTypes[vehicleType.first] = garage.VehicleTypes[vehicleType.first] || vehicleType.second;

                for (const auto& whiteListedVehicle : slot.WhiteListedVehicles)
                    garage.WhiteListedVehicles.insert(whiteListedVehicle);

                for (const auto& blackListedVehicle : slot.BlackListedVehicles)
                    garage.BlackListedVehicles.insert(blackListedVehicle);

                garage.Slots.push_back(slot);
                si++;
            }

            garage.Position = {
                Average(slotPositionsX),
                Average(slotPositionsY),
                Average(slotPositionsZ)
            };

            if (garage.Slots.empty())
                RedLogger::Warning("Garage {} has no slots", garage.Name);
            else
                garages.push_back(garage);

            i++;
        }
    }

    return garages;
}
}
