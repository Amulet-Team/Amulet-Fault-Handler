#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_module(py::module m)
{

}

PYBIND11_MODULE(_faulthandler, m)
{
    m.def("init", &init_module, py::arg("m"));
}
