#include "faulthandler.hpp"

#ifdef _WIN32

#include <windows.h>
#include <dbghelp.h>
#include <werapi.h>

#include <mutex>
#include <optional>
#include <string>
#include <filesystem>
#include <stdexcept>

#include "wer.hpp"

static std::mutex _lock;
static DumpConfig _dump_config;

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
    if (!_dump_config.is_set()) {
        return 0;
    }

    auto* args = static_cast<CrashStack*>(param);

    // Create the path
    wchar_t path[MAX_PATH + 100];
    swprintf_s(
        path,
        L"%ls_%u%ls",
        _dump_config.path,
        _dump_config.dump_index++,
        _dump_config.ext);

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
        MINIDUMP_EXCEPTION_INFORMATION info{};
        info.ThreadId = args->crashing_thread_id;
        info.ExceptionPointers = args->exc_info;
        info.ClientPointers = TRUE;

        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            dump_handle,
            _dump_config.full_dump ? MiniDumpWithFullMemory : MiniDumpNormal,
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

void RegisterWerModule()
{
    // Get the path to the wer module
    HMODULE mod = NULL;
    if (
        GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&RegisterWerModule), &mod
        ) == 0
    ){
        DWORD ret = GetLastError();
        throw std::runtime_error("GetModuleHandleExW failed: " + std::to_string(ret));
    }
    wchar_t fault_handler_path[MAX_PATH];
    if (
        GetModuleFileNameW(mod, fault_handler_path, MAX_PATH) == 0
    ){
        DWORD ret = GetLastError();
        throw std::runtime_error("GetModuleFileNameW failed: " + std::to_string(ret));
    }
    auto wer_path = std::filesystem::path(fault_handler_path).parent_path() / "amulet_wer.dll";

    // Register the WER module
    HRESULT hr = WerRegisterRuntimeExceptionModule(
        wer_path.c_str(),
        reinterpret_cast<PDWORD64>(&_dump_config));

    if (FAILED(hr)){
        throw std::runtime_error("WerRegisterRuntimeExceptionModule failed: " + std::to_string(hr));
    }
}

namespace Amulet {
namespace faulthandler {
    void install(std::filesystem::path path, bool full_dump)
    {
        std::lock_guard lock(_lock);
        if (_dump_config.is_set()) {
            throw std::runtime_error("Faulthandler is already installed");
        }
        auto ext = path.extension();

        _dump_config.set(
            path.replace_extension().c_str(),
            ext.c_str(),
            full_dump
        );
        AddVectoredExceptionHandler(1, exc_handler);
        RegisterWerModule();
    }
} // namespace faulthandler
} // namespace Amulet

#else

namespace Amulet {
namespace faulthandler {
    void install(std::filesystem::path path, bool full_dump)
    {
    }
} // namespace faulthandler
} // namespace Amulet

#endif
