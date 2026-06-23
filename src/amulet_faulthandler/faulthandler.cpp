#include "faulthandler.hpp"

#if _WIN32

#include <windows.h>

#include <dbghelp.h>

#include <mutex>
#include <optional>
#include <string>

struct DumpConfig {
    std::wstring path;
    std::wstring ext;
    bool full_dump;
    std::uint16_t dump_index;
};

static std::mutex _lock;
static std::optional<DumpConfig> _dump_config;

struct CrashStack {
    DWORD code;
    EXCEPTION_POINTERS* exc_info;
    DWORD crashing_thread_id;
    HANDLE done_event;
    HANDLE thread;
};

thread_local static CrashStack _crash_stack;

static DWORD WINAPI dump_thread(LPVOID param) {
    std::lock_guard lock(_lock);
    if (!_dump_config) {
        return 0;
    }

    auto* args = static_cast<CrashStack*>(param);

    // Create the path
    wchar_t path[MAX_PATH + 100];
    swprintf_s(
        path,
        L"%ls_%u%ls",
        _dump_config->path.c_str(),
        _dump_config->dump_index++,
        _dump_config->ext.c_str());

    // Open the file
    HANDLE dump_handle = CreateFileW(
        path,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (dump_handle != INVALID_HANDLE_VALUE) {
        // Write the dump
        MINIDUMP_EXCEPTION_INFORMATION info;
        info.ThreadId = args->crashing_thread_id;
        info.ExceptionPointers = args->exc_info;
        info.ClientPointers = TRUE;

        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            dump_handle,
            _dump_config->full_dump ? MiniDumpWithFullMemory : MiniDumpNormal,
            &info,
            nullptr,
            nullptr);
        // Close the file
        CloseHandle(dump_handle);
    }

    SetEvent(args->done_event);
    return 0;
}

static LONG WINAPI exc_handler(EXCEPTION_POINTERS* exc_info) noexcept
{
    // Get the crash code
    _crash_stack.code = exc_info->ExceptionRecord->ExceptionCode;
    // Skip some errors and non-errors
    if (_crash_stack.code & 0x80000000 && _crash_stack.code != 0xE06D7363 && _crash_stack.code != 0xE0434352 && _crash_stack.code != STATUS_GUARD_PAGE_VIOLATION) {
        _crash_stack.exc_info = exc_info;
        _crash_stack.crashing_thread_id = GetCurrentThreadId();
        _crash_stack.done_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        _crash_stack.thread = CreateThread(nullptr, 0, dump_thread, &_crash_stack, 0, nullptr);
        if (_crash_stack.thread) {
            WaitForSingleObject(_crash_stack.done_event, 30000);
            CloseHandle(_crash_stack.thread);
        }
        CloseHandle(_crash_stack.done_event);
    }
    // Continue to the next handler
    return EXCEPTION_CONTINUE_SEARCH;
}

namespace Amulet {
namespace faulthandler {
    void install(std::filesystem::path path, bool full_dump)
    {
        std::lock_guard lock(_lock);
        if (_dump_config) {
            throw std::runtime_error("Faulthandler is already installed");
        }
        _dump_config.emplace();
        _dump_config->path = path.replace_extension().wstring();
        _dump_config->ext = path.extension().wstring();
        if (_dump_config->ext.empty()) {
            _dump_config->ext = L".dmp";
        }
        if (MAX_PATH < _dump_config->path.size() + _dump_config->ext.size() + 6) {
            _dump_config = std::nullopt;
            throw std::runtime_error("Path is too long for Windows");
        }
        _dump_config->full_dump = full_dump;
        _dump_config->dump_index = 0;
        AddVectoredExceptionHandler(1, exc_handler);
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
