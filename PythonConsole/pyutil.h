#ifndef PYUTIL_H
#define PYUTIL_H

#include "IncludePybind11.h"

namespace pyutil {
void printDict(const py::dict& dict);
void printObjName(const py::object& obj);
void printAttr(const py::object& obj);
void printType(const py::object& obj);

py::str                    toPyStr(const py::handle& hdl);
std::optional<std::string> toStdStr(const py::handle& hdl);
py::list                   getAllAttr(const py::object& obj);
py::object                 getAttr(pybind11::handle handle, const std::string& attrname);
bool                       hasAttr(pybind11::handle handle, const std::string& attrname);
bool                       hasAttr(pybind11::handle handle, const char* attrname);

bool isCallable(const py::object& obj);

/**
 * @brief Is inherit PyTypeObject ?
 * @param obj
 * @return
 */
bool isClassInstance(const py::object& obj);

bool isString(const py::object& obj);

}    // namespace pyutil

#endif    // PYUTIL_H
