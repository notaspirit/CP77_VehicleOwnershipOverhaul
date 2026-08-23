#pragma once

#include <RED4ext/RED4ext.hpp>

namespace RealisticVehicleCallSystem
{
    extern RED4ext::PluginHandle g_pHandle;
    extern const RED4ext::Sdk* g_sdk;
    inline constexpr bool g_isDebug = false;
}