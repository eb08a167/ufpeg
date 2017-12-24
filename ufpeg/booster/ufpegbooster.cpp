#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "executor.hpp"
#include "compiler.hpp"

std::u32string to_u32string(PyObject *pytext) {
    if (!PyUnicode_Check(pytext)) {
        PyErr_Format(PyExc_TypeError, "%R is not a string", pytext);
        return {};
    }

    std::shared_ptr<Py_UCS4> pytext_data(PyUnicode_AsUCS4Copy(pytext), PyMem_Free);
    if (PyErr_Occurred()) {
        return {};
    }

    try {
        return {
            (char32_t*)pytext_data.get(),
            (std::size_t)PyUnicode_GetLength(pytext),
        };
    } catch (std::bad_alloc&) {
        PyErr_NoMemory();
        return {};
    }
}

PyObject *run(PyObject *self, PyObject *args) {
    PyObject *pygrammar, *pytext;

    if (!PyArg_ParseTuple(args, "UU", &pygrammar, &pytext)) {
        return nullptr;
    }

    std::u32string grammar = to_u32string(pygrammar);
    std::u32string text = to_u32string(pytext);
    if (PyErr_Occurred()) {
        return nullptr;
    }

    std::vector<std::shared_ptr<ufpeg::Expression>> choices = {
        std::make_shared<ufpeg::LiteralExpression>(U"foo"),
        std::make_shared<ufpeg::LiteralExpression>(U"bar"),
    };
    auto choice = std::make_shared<ufpeg::SequenceExpression>(choices);
    auto repeat = std::make_shared<ufpeg::RepeatExpression>(choice);
    ufpeg::Compiler compiler;
    compiler.compile(repeat);

    Py_RETURN_NONE;
}

PyMODINIT_FUNC PyInit_booster() {
    static PyMethodDef methods[] = {
        { "run", run, METH_VARARGS, nullptr },
        { nullptr },
    };

    static PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "ufpeg.booster",
        nullptr,
        0,
        methods,
    };

    return PyModule_Create(&moduledef);
}
