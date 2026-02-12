#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(_test_amulet_faulthandler, m)
{
    m.def(
        "throw_access_violation",
        [](){
            int* p = nullptr;
            *p = 0;
        });
}
