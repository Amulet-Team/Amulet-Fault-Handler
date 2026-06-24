#include <pybind11/pybind11.h>

#ifdef _WIN32
#include <windows.h>
#endif

namespace py = pybind11;

void throw_access_violation(){
    int* p = nullptr;
    *p = 0;
}

#pragma optimize("", off)
void throw_stack_overflow(){
    throw_stack_overflow();
}
#pragma optimize("", on)

#pragma optimize("", off)
void throw_heap_corruption(){
#ifdef _WIN32
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
#endif
    char* cp = new char[32];
    for (int i = 1; i <= 64; i++) {
        *(cp - i) = 0xFF;
        *(cp + 32 + i) = 0xFF;
    }
    delete[] cp;
    delete[] cp;
}
#pragma optimize("", on)

PYBIND11_MODULE(_test_amulet_faulthandler, m)
{
    m.def("throw_access_violation", throw_access_violation);
    m.def("throw_stack_overflow", throw_stack_overflow);
    m.def("throw_heap_corruption", throw_heap_corruption);
}
