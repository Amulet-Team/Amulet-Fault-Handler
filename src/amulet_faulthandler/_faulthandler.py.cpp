#include <pybind11/pybind11.h>
#include <pybind11/stl/filesystem.h>

#include <amulet_faulthandler/faulthandler.hpp>

namespace py = pybind11;

void init_module(py::module m)
{
    m.def(
        "install",
        &Amulet::faulthandler::install,
        py::arg("path"));
}

PYBIND11_MODULE(_faulthandler, m)
{
    m.def("init", &init_module, py::arg("m"));
}
