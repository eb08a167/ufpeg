#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "bootstrap.hpp"
#include "compiler.hpp"
#include "executor.hpp"
#include "nodevisitor.hpp"

void dump(const std::u32string &text, const ufpeg::Node &node, std::size_t level = 0) {
    std::cout << std::string(level, '\t') << u32tou8(node.name) << " " << node.start << ":" << node.stop << " " << u32tou8(text.substr(node.start, node.stop - node.start)) << std::endl;

    for (const auto &child: node.children) {
        dump(text, child, level + 1);
    }
}

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

    auto rule = ufpeg::bootstrap();

    ufpeg::Compiler compiler;
    auto instructions = compiler.compile(rule);
    for (auto instruction: instructions) {
        std::cout << "L" << instruction->get_reference()->get_offset() << ": ";
        instruction->print();
    }

    ufpeg::Executor executor(instructions);
    auto node = executor.execute(grammar);

    dump(grammar, node);

    // ufpeg::NodeVisitor<std::shared_ptr<ufpeg::Expression>> expression_visitor;

    // expression_visitor.add_handler(U"foo", [&]() {
    //     return repeat;
    // });

    // expression_visitor.visit(node);

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
