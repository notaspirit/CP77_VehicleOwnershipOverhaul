#include <RED4ext/RED4ext.hpp>
#include <RealisticVehicleCallSystem.h>
#include "DataStructs/Globals.h"

namespace RealisticVehicleCallSystem
{
    RED4ext::PluginHandle g_pHandle;
    const RED4ext::Sdk* g_sdk;

    RED4ext::TTypedClass<RealisticVehicleCallSystem> customControllerClass(
    "RealisticVehicleCallSystemNative");

    RED4ext::CClass* RealisticVehicleCallSystem::GetNativeType()
    {
        return &customControllerClass;
    }

    RED4EXT_C_EXPORT void RED4EXT_CALL RegisterTypes()
    {
        RED4ext::CNamePool::Add("RealisticVehicleCallSystemNative");

        customControllerClass.flags = {.isNative = true};
        RED4ext::CRTTISystem::Get()->RegisterType(&customControllerClass);
    }

    RED4EXT_C_EXPORT void RED4EXT_CALL PostRegisterTypes()
    {
        const auto rtti = RED4ext::CRTTISystem::Get();
        const auto scriptable = rtti->GetClass("IScriptable");
        customControllerClass.parent = scriptable;

        const auto onGeoCachePostLoad =
            RED4ext::CClassStaticFunction::Create(&customControllerClass, "OnGeoCachePostLoad", "OnGeoCachePostLoad",
            &RealisticVehicleCallSystem::OnGeoCachePostLoad, {.isNative = true, .isStatic = true});

        onGeoCachePostLoad->AddParam("handle:physicsGeometryCache", "geoCache");
        customControllerClass.RegisterFunction(onGeoCachePostLoad);

    }

    RED4EXT_C_EXPORT bool RED4EXT_CALL Main(RED4ext::PluginHandle aHandle, RED4ext::EMainReason aReason, const RED4ext::Sdk* aSdk)
    {
        switch (aReason)
        {
            case RED4ext::EMainReason::Load:
            {
                g_pHandle = aHandle;
                g_sdk = aSdk;

                RED4ext::CRTTISystem::Get()->AddRegisterCallback(RegisterTypes);
                RED4ext::CRTTISystem::Get()->AddPostRegisterCallback(PostRegisterTypes);

                g_sdk->logger->Info(g_pHandle, "UnlimitedGeometryCacheStreamingNative loaded");
                break;
            }
            case RED4ext::EMainReason::Unload:
            {
                break;
            }
        }

        return true;
    }

    RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::PluginInfo* aInfo)
    {
        aInfo->name = L"UnlimitedGeometryCacheStreamingNative";
        aInfo->author = L"sprt_";
        aInfo->version = RED4EXT_SEMVER(1, 0, 0);
        aInfo->runtime = RED4EXT_RUNTIME_INDEPENDENT;
        aInfo->sdk = RED4EXT_SDK_LATEST;
    }

    RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
    {
        return RED4EXT_API_VERSION_LATEST;
    }
}