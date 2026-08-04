#pragma once
#include <string>

#include "RapidJson/document.h"
#include "RapidJson/encodings.h"


namespace RealisticVehicleCallSystem
{
class RapidJsonHelper
{
public:
    static bool TryGetStringValue(const rapidjson::GenericValue<rapidjson::UTF8<>>& base, const char* key, std::string& value);
    static bool TryGetFloatValue(const rapidjson::GenericValue<rapidjson::UTF8<>>& base, const char* key, float& value);

    static bool IsObject(const rapidjson::GenericValue<rapidjson::UTF8<>>& base, const char* key);

    static bool IsObjectArray(const rapidjson::GenericValue<rapidjson::UTF8<>>& base, const char* key);
    static bool IsObjectArray(const rapidjson::GenericValue<rapidjson::UTF8<>>& base);
    static bool IsStringArray(const rapidjson::GenericValue<rapidjson::UTF8<>>& base, const char* key);
};
}
