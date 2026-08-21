#pragma once
#include <format>
#include "DataStructs/Globals.h"

namespace RealisticVehicleCallSystem {
    struct RedLogger {
        template <class... _Types>
        static void Info(const std::format_string<_Types...> _Fmt, _Types&&... _Args) {
            g_sdk->logger->Info(g_pHandle, std::format(_Fmt, std::forward<_Types>(_Args)...).c_str());
        }

        template <class... _Types>
        static void Error(const std::format_string<_Types...> _Fmt, _Types&&... _Args) {
            g_sdk->logger->Info(g_pHandle, std::format(_Fmt, std::forward<_Types>(_Args)...).c_str());
        }

        template <class... _Types>
        static void Warning(const std::format_string<_Types...> _Fmt, _Types&&... _Args) {
            g_sdk->logger->Info(g_pHandle, std::format(_Fmt, std::forward<_Types>(_Args)...).c_str());
        }

        template <class... _Types>
        static void Debug(const std::format_string<_Types...> _Fmt, _Types&&... _Args) {
            if constexpr (!g_isDebug) {
                return;
            }
            g_sdk->logger->Info(g_pHandle, std::format(_Fmt, std::forward<_Types>(_Args)...).c_str());
        }
    };
}