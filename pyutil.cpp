#include "pyutil.h"
#include <iostream>
#include "defines.h"

namespace detail {
/**
 * @brief From FreeCAD
 * @param type
 * @return
 */
PyObject* getTypeAsObject(PyTypeObject* type)
{
    PyObject* obj;
    std::memcpy(&obj, &type, sizeof type);
    return obj;
}

/**
 * @brief From PyCXX
 * @param op
 * @param type
 * @return
 */
int _IsInstance(PyObject* op, PyTypeObject* type)
{
    return PyObject_IsInstance(op, reinterpret_cast<PyObject*>(type));
}

/**
 * @brief From PyCXX
 * @return
 */
PyTypeObject* _Unicode_Type()
{
    return &PyUnicode_Type;
}

/**
 * @brief From PyCXX
 * @param op
 * @return
 */
bool _Unicode_Check(PyObject* op)
{
    return _IsInstance(op, _Unicode_Type()) > 0;
}
}    // namespace detail

void pyutil::printDict(const py::dict& dict)
{
    LOG << "======= Print elements of py::dict =======";
    try {
        for (auto&& [key, val] : dict) {
            const auto keystr = py::str(key).cast<std::string>();
            const auto valstr = py::str(val).cast<std::string>();
            LOG << "{" << keystr << "," << valstr << "}";
        }
    }
    catch (py::error_already_set e) {
        WARNLOG << e.what();
    }
    catch (...) {
        WARNLOG << "Unknown error";
    }
    PyErr_Clear();
}

void pyutil::printObjName(const py::object& obj)
{
    try {
        const auto obj_name = py::str(obj).cast<std::string>();
        LOG << VAROUT(obj_name);
    }
    catch (py::error_already_set e) {
        WARNLOG << e.what();
    }
    catch (...) {
        WARNLOG << "Unknown error";
    }
    PyErr_Clear();
}

void pyutil::printAttr(const py::object& obj)
{
    LOG << "======= Print attributes of py::object =======";

    try {
        printObjName(obj);

        py::list attributes = getAllAttr(obj);

        // 属性のリストをループ処理で出力
        for (auto&& attr : attributes) {
            std::string attr_name = py::str(attr).cast<std::string>();
            LOG << VAROUT(attr_name);
        }
    }
    catch (py::error_already_set e) {
        WARNLOG << e.what();
    }
    catch (...) {
        LOG << "Unknown error";
    }
    PyErr_Clear();
}

void pyutil::printType(const py::object& obj)
{
    LOG << "======= Print type of py::object =======";
    try {
        printObjName(obj);

        py::object builtins    = py::module_::import("builtins");
        py::object type_func   = builtins.attr("type");
        py::object type_of_obj = type_func(obj);

        std::string type_name = py::str(type_of_obj).cast<std::string>();

        // 型名を出力
        LOG << VAROUT(type_name);
    }
    catch (py::error_already_set e) {
        WARNLOG << e.what();
    }
    catch (...) {
        WARNLOG << "Unknown error";
    }
    PyErr_Clear();
}

py::str pyutil::toPyStr(const py::handle& hdl)
{
    try {
        return py::str(hdl);
    }
    catch (py::error_already_set e) {
        WARNLOG << e.what();
    }
    catch (...) {
        WARNLOG << "Unknown error";
    }
    PyErr_Clear();
    return py::none{};
}

std::optional<std::string> pyutil::toStdStr(const pybind11::handle& hdl)
{
    auto pystr = toPyStr(hdl);
    if (pystr.is_none()) {
        return std::nullopt;
    }
    else {
        return pystr.cast<std::string>();
    }
}

pybind11::list pyutil::getAllAttr(const pybind11::object& obj)
{
    try {
        py::object builtins = py::module_::import("builtins");
        py::object dir      = builtins.attr("dir");

        // objのすべての属性を取得
        return dir(obj);
    }
    catch (py::error_already_set e) {
        WARNLOG << e.what();
    }
    catch (...) {
        WARNLOG << "Unknown error";
    }
    PyErr_Clear();
    return py::none{};
}

py::object pyutil::getAttr(py::handle handle, const std::string& attrname)
{
    PyErr_Clear();    /// CPythonのAsserに引っかからないようにクリアしておく.
    return py::getattr(handle, attrname.c_str(), py::none{});
}

bool pyutil::hasAttr(pybind11::handle handle, const std::string& attrname)
{
    return hasAttr(handle, attrname.c_str());
}

bool pyutil::hasAttr(pybind11::handle handle, const char* attrname)
{
    PyErr_Clear();    /// CPythonのAsserに引っかからないようにクリアしておく.
    if (handle.is_none()) {
        return false;
    }
    return py::hasattr(handle, attrname);
}

bool pyutil::isCallable(const py::object& obj)
{
    return PyCallable_Check(obj.ptr()) == 1;
}

bool pyutil::isClassInstance(const py::object& obj)
{
    if (obj.is_none()) {
        return false;
    }
    try {
        PyObject* basetype = detail::getTypeAsObject(&PyBaseObject_Type);
        return (PyObject_IsSubclass(obj.ptr(), basetype) == 1);
    }
    catch (py::error_already_set e) {
        WARNLOG << e.what();
    }
    catch (...) {
        WARNLOG << "Unknown error";
    }
    PyErr_Clear();
    return false;
}

bool pyutil::isString(const py::object& obj)
{
    if (obj.is_none()) {
        return false;
    }
    else {
        return detail::_Unicode_Check(obj.ptr());
    }
}
