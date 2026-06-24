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

void throw_heap_corruption(){
    int* p = new int;
    delete p;
    delete p;

    char *cp = new char[10];
    (*(cp - 5))++;
    delete cp;
}
#pragma optimize("", on)

PYBIND11_MODULE(_test_amulet_faulthandler, m)
{
    m.def("throw_access_violation", throw_access_violation);
    m.def("throw_stack_overflow", throw_stack_overflow);
    m.def("throw_heap_corruption", throw_heap_corruption);
}
