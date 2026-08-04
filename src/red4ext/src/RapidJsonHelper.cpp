//
// Created by zweit on 04/08/2026.
//

#include "RapidJsonHelper.h"

namespace RealisticVehicleCallSystem
{
bool RapidJsonHelper::TryGetStringValue(const rapidjson::GenericValue<rapidjson::UTF8<>>& base,
    const char* key,
    std::string& value)
{
    if (!base.HasMember(key) || !base[key].IsString())
        return false;
    value = base[key].GetString();
    return true;
}

bool RapidJsonHelper::TryGetFloatValue(const rapidjson::GenericValue<rapidjson::UTF8<>> &base, const char *key,
    float &value)
{
    if (!base.HasMember(key) || !base[key].IsFloat())
        return false;
    value = base[key].GetFloat();
    return true;
}

bool RapidJsonHelper::IsObject(const rapidjson::GenericValue<rapidjson::UTF8<>> &base,
                               const char *key)
{
    if (!base.HasMember(key) || !base[key].IsObject())
        return false;

    return true;
}

bool RapidJsonHelper::IsObjectArray(const rapidjson::GenericValue<rapidjson::UTF8<>>& base,
                                    const char* key)
{
    if (!base.HasMember(key) || !base[key].IsArray())
        return false;

    for (const auto& item : base[key].GetArray())
    {
        if (!item.IsObject())
            return false;
    }

    return true;
}

bool RapidJsonHelper::IsObjectArray(const rapidjson::GenericValue<rapidjson::UTF8<>>& base)
{
    if (!base.IsArray())
        return false;

    for (const auto& item : base.GetArray())
    {
        if (!item.IsObject())
            return false;
    }

    return true;
}

bool RapidJsonHelper::IsStringArray(const rapidjson::GenericValue<rapidjson::UTF8<>> &base,
    const char *key)
{
    if (!base.HasMember(key) || !base[key].IsArray())
        return false;

    for (const auto& item : base[key].GetArray())
    {
        if (!item.IsString())
            return false;
    }

    return true;
}


}
