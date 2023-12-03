
/// QtとPythonでマクロ slots の名前が競合しているので回避
/// https://stackoverflow.com/questions/23068700/embedding-python3-in-qt-5
#pragma push_macro("slots")
#undef slots
#include <pybind11/embed.h>
namespace py = pybind11;
#pragma pop_macro("slots")

/// https://github.com/pybind/pybind11/issues/1622
class PyStdErrOutStreamRedirect {
    py::object _stdout;
    py::object _stderr;
    py::object _stdout_buffer;
    py::object _stderr_buffer;

public:
    PyStdErrOutStreamRedirect()
    {
        auto sysm = py::module::import("sys");
        _stdout   = sysm.attr("stdout");
        _stderr   = sysm.attr("stderr");

        // Other filelike object can be used here as well, such as objects created by pybind11
        auto stringio  = py::module::import("io").attr("StringIO");
        _stdout_buffer = stringio();
        _stderr_buffer = stringio();

        sysm.attr("stdout") = _stdout_buffer;
        sysm.attr("stderr") = _stderr_buffer;
    }

    ~PyStdErrOutStreamRedirect()
    {
        auto sysm           = py::module::import("sys");
        sysm.attr("stdout") = _stdout;
        sysm.attr("stderr") = _stderr;
    }

    std::string stdoutString()
    {
        _stdout_buffer.attr("seek")(0);
        return py::str(_stdout_buffer.attr("read")());
    }
    std::string stderrString()
    {
        _stderr_buffer.attr("seek")(0);
        return py::str(_stderr_buffer.attr("read")());
    }
    void stdoutRefresh() { _stdout_buffer.attr("truncate")(0); }
    void stderrRefresh() { _stderr_buffer.attr("truncate")(0); }
};
