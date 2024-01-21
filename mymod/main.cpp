#include <pybind11/pybind11.h>

#include "CadFileReader.h"

namespace py = pybind11;

int add(int i, int j)
{
    return i + j;
}

int add_default(int i = 1, int j = 2)
{
    return i + j;
}

PYBIND11_MODULE(mymod, m)
{
    /// optional module docstring
    m.doc() = "pybind11 example module";

    ///---------------------------------------------------------
    /// 関数のExport
    ///---------------------------------------------------------
    m.def("add", &add, "A function that adds two numbers");

    /// キーワード引数
    m.def("add1", &add, py::arg("i"), py::arg("j"));

    /// 短縮形
    using namespace pybind11::literals;
    m.def("add2", &add, "i"_a, "j"_a);

    /// デフォルト引数付き
    m.def("add_default", &add_default, "i"_a = 1, "j"_a = 2);

    ///---------------------------------------------------------
    /// 変数のExport
    ///---------------------------------------------------------
    m.attr("the_answer") = 42;
    py::object world     = py::cast("World");
    m.attr("what")       = world;

    ///---------------------------------------------------------
    /// クラスのExport
    ///---------------------------------------------------------
    py::class_<CadFileReader>(m, "CadFileReader")
        .def(py::init<>())
        .def("readStep", &CadFileReader::readStep)
        .def("readStl", &CadFileReader::readStl);
}
