#include "faulthandler.hpp"

#if WIN32

#include <windows.h>

#include <dbghelp.h>

#include <string>

static std::wstring _path;
static bool _full_dump = false;

void dump_process(EXCEPTION_POINTERS* p)
{
    HANDLE dump_handle = CreateFileW(
        _path.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (dump_handle != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION info;
        info.ThreadId = GetCurrentThreadId();
        info.ExceptionPointers = p;
        info.ClientPointers = TRUE;

        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            dump_handle,
            _full_dump ? MiniDumpWithFullMemory : MiniDumpNormal,
            &info,
            nullptr,
            nullptr);
        CloseHandle(dump_handle);
    }
}

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* p)
{
    dump_process(p);
    return EXCEPTION_EXECUTE_HANDLER;
}

namespace Amulet {
namespace faulthandler {
    void install(std::filesystem::path path, bool full_dump)
    {
        _path = path.wstring();
        _full_dump = full_dump;
        SetUnhandledExceptionFilter(CrashHandler);
    }
}
}

#else

namespace Amulet {
namespace faulthandler {
    void install(std::filesystem::path path, bool full_dump)
    {
    }
}
}

#endif
