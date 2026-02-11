#include "faulthandler.hpp"

#if WIN32

#include <windows.h>

#include <dbghelp.h>

#include <memory>
#include <string>

// https://stackoverflow.com/questions/22467604/how-can-you-use-capturestackbacktrace-to-capture-the-exception-stack-not-the-ca

const int MaxNameLen = 256;

void dump_stack(EXCEPTION_POINTERS* p)
{
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();
    SymInitialize(process, NULL, TRUE);
    CONTEXT ctx = *p->ContextRecord;
    STACKFRAME64 frame {};

#if defined(_M_AMD64)
    frame.AddrPC.Offset = ctx.Rip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Offset = ctx.Rsp;
    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = ctx.Rbp;
    frame.AddrFrame.Mode = AddrModeFlat;
#define MACHINE IMAGE_FILE_MACHINE_AMD64

#elif defined(_M_IX86)
    frame.AddrPC.Offset = ctx.Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Offset = ctx.Esp;
    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = ctx.Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
#define MACHINE IMAGE_FILE_MACHINE_I386

#else
#error "Unsupported architecture"

#endif

    printf("*** Exception 0x%x occured ***\n", p->ExceptionRecord->ExceptionCode);

    while (StackWalk64(
        MACHINE, process, thread, &frame, &ctx, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
        // get symbol name for address
        char symbol_buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(symbol_buffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;
        DWORD64 displacement = 0;
        if (!SymFromAddr(process, frame.AddrPC.Offset, &displacement, symbol)) {
            strcpy_s(symbol->Name, MAX_SYM_NAME, "<unknown>");
        }

        IMAGEHLP_LINE64 line = { sizeof(IMAGEHLP_LINE64) };

        // try to get line
        DWORD disp;
        if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &disp, &line)) {
            printf("in %s\n\tat %s, line: %lu, address: 0x%llx\n", line.FileName, symbol->Name, line.LineNumber, symbol->Address);
        } else {
            // failed to get line
            HMODULE module_handle = NULL;
            char module_name[MaxNameLen] = {};
            GetModuleHandleExA(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCTSTR>(frame.AddrPC.Offset),
                &module_handle);

            // at least print module name
            if (module_handle != NULL)
                GetModuleFileNameA(module_handle, module_name, MaxNameLen);

            printf("in %s\n\tat %s, address 0x%llx.\n", module_name, symbol->Name, symbol->Address);
        }
    }
}

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
    dump_stack(p);
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
