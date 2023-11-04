#include <Python.h>

#include <CXX/Objects.hxx>

int main(int argc, char *argv[])
{
    // Initialize the Python interpreter
    Py_Initialize();

    // Create a Python list
    PyObject *pList = PyList_New(0);
    if (pList == NULL) {
        return 1;    // Error creating Python list
    }

    // Add integers to the list
    for (int i = 1; i <= 5; ++i) {
        PyObject *pValue = PyLong_FromLong(i);
        if (pValue == NULL) {
            Py_DECREF(pList);
            return 1;    // Error creating Python integer
        }
        PyList_Append(pList, pValue);
        Py_DECREF(pValue);
    }

    // Import Python's built-in "builtins" module
    PyObject *pBuiltins = PyImport_ImportModule("builtins");
    if (pBuiltins == NULL) {
        Py_DECREF(pList);
        return 1;    // Error importing module
    }

    // Get the 'sum' function from the builtins module
    PyObject *pSumFunc = PyObject_GetAttrString(pBuiltins, "sum");
    if (pSumFunc == NULL || !PyCallable_Check(pSumFunc)) {
        Py_XDECREF(pSumFunc);
        Py_DECREF(pBuiltins);
        Py_DECREF(pList);
        return 1;    // Error getting 'sum' function
    }

    // Call the 'sum' function
    PyObject *pArgs   = PyTuple_Pack(1, pList);
    PyObject *pResult = PyObject_CallObject(pSumFunc, pArgs);
    if (pResult == NULL) {
        Py_DECREF(pArgs);
        Py_DECREF(pSumFunc);
        Py_DECREF(pBuiltins);
        Py_DECREF(pList);
        return 1;    // Error calling 'sum' function
    }

    // Print the result
    long sum = PyLong_AsLong(pResult);
    printf("The sum is: %ld\n", sum);

    // Clean up
    Py_DECREF(pResult);
    Py_DECREF(pArgs);
    Py_DECREF(pSumFunc);
    Py_DECREF(pBuiltins);
    Py_DECREF(pList);

    ///--------------------------------------------------------
    ///--------------------------------------------------------
    ///--------------------------------------------------------
    PyRun_SimpleString("import os, sys \n"
                       "sys.path.append(os.getcwd()) \n");

    FILE *fp = fopen("called_from_cpp.py", "r");
    if (fp == nullptr) {
        fprintf(stderr, "Error: cannot open file.");
        return 1;
    }

    PyRun_SimpleFile(fp, "called_from_cpp.py");

    fclose(fp);

    // Finalize the Python interpreter
    Py_Finalize();

    return 0;
}
