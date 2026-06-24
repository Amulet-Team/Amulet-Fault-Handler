#include <pybind11/pybind11.h>

#if _WIN32
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

void throw_heap_corruption(){

#if _WIN32
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
#endif

    int* p = new int;
    delete p;
    delete p;
}
#pragma optimize("", on)

PYBIND11_MODULE(_test_amulet_faulthandler, m)
{
    m.def("throw_access_violation", throw_access_violation);
    m.def("throw_stack_overflow", throw_stack_overflow);
    m.def("throw_heap_corruption", throw_heap_corruption);
}
