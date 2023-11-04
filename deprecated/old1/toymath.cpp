#define PY_SSIZE_T_CLEAN

#include <Python.h>

static int add(int a, int b)
{
    return a + b;
}

/**
 * @brief C++関数add()をラップする.
 * @param self
 * @param args
 * @return
 */
static PyObject* toymath_add(PyObject* self, PyObject* args)
{
    int a, b;
    if (!PyArg_ParseTuple(args, "ii", &a, &b)) {
        return nullptr;
    }
    int ret = add(a, b);
    return PyLong_FromLong(ret);
}

static PyMethodDef toymath_methods[] = {
    {"add",   toymath_add, METH_VARARGS, "Add integer values"},
    {nullptr, nullptr,     0,            nullptr             },
};

static PyModuleDef toymath_module = {PyModuleDef_HEAD_INIT, "toymath", "Toy mathematic functions.", -1,
                                     toymath_methods};

/**
 * モジュールの初期化
 * importされたときに呼ばれる
 * PyInit_XXXのXXXはモジュール名と一致させる
 */
PyMODINIT_FUNC PyInit_toymath()
{
    return PyModule_Create(&toymath_module);
}
