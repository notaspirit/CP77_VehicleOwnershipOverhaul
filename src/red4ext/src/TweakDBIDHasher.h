#pragma once
#include "RED4ext/NativeTypes.hpp"

namespace RealisticVehicleCallSystem
{
struct TweakDBIDHasher
{
    size_t operator()(const RED4ext::TweakDBID& id) const noexcept
    {
        return id.value;
    }
};
}
