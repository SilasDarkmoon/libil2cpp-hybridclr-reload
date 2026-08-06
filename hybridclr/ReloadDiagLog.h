#pragma once

// ==={{ AssemblyReloadDiag
// Header-only logger for assembly-reload diagnostics.
//
// Dual output:
//   1. our own file, flushed per line (survives hard crashes)
//   2. il2cpp::utils::Logging::Write -> Unity's log callback, so the trail
//      is also captured by Unity's own log / crash reporting.
//
// File path resolution (first hit wins):
//   1. $RELOAD_DIAG_LOG_PATH              (full file path, settable from C#
//      via System.Environment.SetEnvironmentVariable before first use)
//   2. $UNITY_TEMPORARY_CACHE_PATH/reload_diag.log
//   3. reload_diag.log                    (current working directory)
//
// Only standard C/C++ is used so it compiles and runs on Windows editor,
// Android and iOS. Each call opens/appends/closes so output survives
// crashes; call sites are heavily filtered so the overhead is acceptable.

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <ctime>

#include "utils/Logging.h"

namespace hybridclr
{
    // Some diagnostics resolve classes (GetTypeInfoFromType / FromIl2CppType)
    // for name filtering, which can trigger nested class creation. That is
    // unsafe during VM startup (e.g. inside GenericClass::CreateClass while
    // core generic types are being created). Keep such diagnostics disabled
    // until the first assembly reload begins; RestoreReusedClasses enables
    // them via ReloadDiagEnable().
    inline bool& ReloadDiagEnabledFlag()
    {
        static bool s_Enabled = false;
        return s_Enabled;
    }

    inline bool ReloadDiagEnabled()
    {
        return ReloadDiagEnabledFlag();
    }

    inline void ReloadDiagEnable()
    {
        ReloadDiagEnabledFlag() = true;
    }

    // Re-entrancy guard. Diagnostics that resolve classes can trigger nested
    // class / generic-instance creation, which re-enters the same diagnostic
    // code and may recurse without bound. Nested diag invocations are
    // skipped, breaking the recursion.
    inline bool& ReloadDiagInProgressFlag()
    {
        static thread_local bool s_InProgress = false;
        return s_InProgress;
    }

    inline bool ReloadDiagTryEnter()
    {
        if (ReloadDiagInProgressFlag())
            return false;
        ReloadDiagInProgressFlag() = true;
        return true;
    }

    inline void ReloadDiagLeave()
    {
        ReloadDiagInProgressFlag() = false;
    }

    inline const char* ReloadDiagLogPath()
    {
        static char s_Path[1088] = { 0 };
        if (s_Path[0] != '\0')
            return s_Path;

        const char* explicitPath = std::getenv("RELOAD_DIAG_LOG_PATH");
        if (explicitPath != NULL && explicitPath[0] != '\0')
        {
            std::snprintf(s_Path, sizeof(s_Path), "%s", explicitPath);
            return s_Path;
        }

        const char* baseDir = std::getenv("UNITY_TEMPORARY_CACHE_PATH");
        if (baseDir != NULL && baseDir[0] != '\0')
            std::snprintf(s_Path, sizeof(s_Path), "%s/reload_diag.log", baseDir);
        else
            std::snprintf(s_Path, sizeof(s_Path), "reload_diag.log");
        return s_Path;
    }

    inline void ReloadDiagLog(const char* format, ...)
    {
        char body[2048];
        va_list args;
        va_start(args, format);
        std::vsnprintf(body, sizeof(body), format, args);
        va_end(args);
        body[sizeof(body) - 1] = '\0';

        char timeBuf[32] = "?";
        time_t now = time(NULL);
        struct tm* utc = std::gmtime(&now);
        if (utc != NULL)
            std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", utc);

        char line[2240];
        std::snprintf(line, sizeof(line), "[%s UTC] %s", timeBuf, body);
        line[sizeof(line) - 1] = '\0';

        // 1) own file, flushed per line so the trail survives a hard crash.
        FILE* f = std::fopen(ReloadDiagLogPath(), "a");
        if (f != NULL)
        {
            std::fputs(line, f);
            std::fflush(f);
            std::fclose(f);
        }

        // 2) Unity's own log via the il2cpp log callback, so the message is
        // also captured in Unity's log file / crash report.
        if (il2cpp::utils::Logging::IsLogCallbackSet())
            il2cpp::utils::Logging::Write("%s", line);
    }
}
// ===}} AssemblyReloadDiag
