#pragma once

#include <RED4ext/RED4ext.hpp>

#include "FastRNG.h"

namespace RealisticVehicleCallSystem
{
    extern RED4ext::PluginHandle g_pHandle;
    extern const RED4ext::Sdk* g_sdk;
    static inline auto g_rng = FastRNG();
    inline constexpr bool g_isDebug = false;
}