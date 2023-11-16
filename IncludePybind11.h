#ifndef INCLUDEPYBIND11_H
#define INCLUDEPYBIND11_H

/// QtとPythonでマクロ slots の名前が競合しているので回避
/// https://stackoverflow.com/questions/23068700/embedding-python3-in-qt-5

#pragma push_macro("slots")
#undef slots
#include <pybind11/embed.h>
#pragma pop_macro("slots")

namespace py = pybind11;

#endif    // INCLUDEPYBIND11_H
