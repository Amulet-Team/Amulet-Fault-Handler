#include <windows.h>
#include <dbghelp.h>
#include <werapi.h>

#include "wer.hpp"

extern "C" __declspec(dllexport) HRESULT CALLBACK OutOfProcessExceptionEventCallback(
    _In_    PVOID pContext,
    _In_    const PWER_RUNTIME_EXCEPTION_INFORMATION pExceptionInformation,
    _Inout_ BOOL* pbOwnershipClaimed,
    _Out_writes_(*pchSize) PWSTR pwszEventName,
    _Inout_ PDWORD pchSize,
    _Out_   PDWORD pdwSignatureCount)
{
    DumpConfig dump_config{};
    size_t bytesRead = 0;
    if (
        !ReadProcessMemory(
            pExceptionInformation->hProcess,
            pContext,
            &dump_config,
            sizeof(dump_config),
            &bytesRead
        )
        || bytesRead != sizeof(dump_config)
        || !dump_config.is_dump_context()
        || !dump_config.is_set()) {
        return S_OK;
    }

    // Create the path
    wchar_t path[MAX_PATH + 100];
    swprintf_s(
        path,
        L"%ls_%u%ls",
        dump_config.path,
        dump_config.dump_index++,
        dump_config.ext);

    // Open the file
    HANDLE dump_handle = CreateFileW(
        path,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (dump_handle != INVALID_HANDLE_VALUE)
    {
        EXCEPTION_POINTERS exPtrs{};
        exPtrs.ExceptionRecord = &pExceptionInformation->exceptionRecord;
        exPtrs.ContextRecord = &pExceptionInformation->context;

        // Write the dump
        MINIDUMP_EXCEPTION_INFORMATION info{};
        info.ThreadId = GetThreadId(pExceptionInformation->hThread);
        info.ExceptionPointers = &exPtrs;
        info.ClientPointers = FALSE;

        MiniDumpWriteDump(
            pExceptionInformation->hProcess,
            GetProcessId(pExceptionInformation->hProcess),
            dump_handle,
            dump_config.full_dump ? MiniDumpWithFullMemory : MiniDumpNormal,
            &info,
            nullptr,
            nullptr);
        // Close the file
        CloseHandle(dump_handle);
    }

    *pbOwnershipClaimed = FALSE;

    const wchar_t* eventName = L"AmuletFaultHandler";
    size_t len = wcslen(eventName);
    if (*pchSize > len)
    {
        wcscpy_s(pwszEventName, *pchSize, eventName);
    }
    *pchSize = (DWORD)(len + 1);

    *pdwSignatureCount = 0;

    return S_OK;
}

extern "C" __declspec(dllexport) HRESULT CALLBACK OutOfProcessExceptionEventSignatureCallback(
    _In_    PVOID pContext,
    _In_    const PWER_RUNTIME_EXCEPTION_INFORMATION pExceptionInformation,
    _In_    DWORD dwIndex,
    _Out_writes_(*pchName) PWSTR pwszName,
    _Inout_ PDWORD pchName,
    _Out_writes_(*pchValue) PWSTR pwszValue,
    _Inout_ PDWORD pchValue)
{
    (void)pContext; (void)pExceptionInformation; (void)dwIndex;
    (void)pwszName; (void)pwszValue;
    *pchName = 0;
    *pchValue = 0;
    return S_OK;
}

extern "C" __declspec(dllexport) HRESULT CALLBACK OutOfProcessExceptionEventDebuggerLaunchCallback(
    _In_    PVOID pContext,
    _In_    const PWER_RUNTIME_EXCEPTION_INFORMATION pExceptionInformation,
    _Out_   PBOOL pbIsCustomDebugger,
    _Out_writes_(*pchCmdLine) PWSTR pwszCmdLine,
    _Inout_ PDWORD pchCmdLine,
    _Out_   PBOOL pbIsSecondChanceDebugger)
{
    (void)pContext; (void)pExceptionInformation; (void)pwszCmdLine;
    *pbIsCustomDebugger = FALSE;
    *pchCmdLine = 0;
    *pbIsSecondChanceDebugger = FALSE;
    return S_OK;
}

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID)
{
    return TRUE;
}
