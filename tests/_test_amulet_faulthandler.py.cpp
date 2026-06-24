#include <pybind11/pybind11.h>

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

void throw_heap_corruption(){
    char *cp = new char[10];
    (*(cp - 5))++;
    free(cp);
}

PYBIND11_MODULE(_test_amulet_faulthandler, m)
{
    m.def("throw_access_violation", throw_access_violation);
    m.def("throw_stack_overflow", throw_stack_overflow);
    m.def("throw_heap_corruption", throw_heap_corruption);
}
