#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <functional>
#include <map>

#include "vm.hpp"

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

ufpeg::InvokeInstruction *to_invoke_instruction(PyObject *pyinstruction) {
    std::shared_ptr<PyObject> pypointer = {
        PyObject_GetAttrString(pyinstruction, "pointer"),
        Py_DecRef,
    };
    if (PyErr_Occurred()) {
        return nullptr;
    }

    auto pointer = PyLong_AsSize_t(pypointer.get());
    if (PyErr_Occurred()) {
        return nullptr;
    } else {
        return new ufpeg::InvokeInstruction(pointer);
    }
}

ufpeg::RevokeInstruction *to_revoke_instruction(PyObject*) {
    return new ufpeg::RevokeInstruction();
}

ufpeg::PrepareInstruction *to_prepare_instruction(PyObject*) {
    return new ufpeg::PrepareInstruction();
}

ufpeg::ConsumeInstruction *to_consume_instruction(PyObject *pyinstruction) {
    std::shared_ptr<PyObject> pyname = {
        PyObject_GetAttrString(pyinstruction, "name"),
        Py_DecRef,
    };
    if (PyErr_Occurred()) {
        return nullptr;
    }

    auto name = to_u32string(pyname.get());
    if (PyErr_Occurred()) {
        return nullptr;
    } else {
        return new ufpeg::ConsumeInstruction(name);
    }
}

ufpeg::DiscardInstruction *to_discard_instruction(PyObject*) {
    return new ufpeg::DiscardInstruction();
}

ufpeg::BeginInstruction *to_begin_instruction(PyObject*) {
    return new ufpeg::BeginInstruction();
}

ufpeg::CommitInstruction *to_commit_instruction(PyObject*) {
    return new ufpeg::CommitInstruction();
}

ufpeg::AbortInstruction *to_abort_instruction(PyObject*) {
    return new ufpeg::AbortInstruction();
}

ufpeg::MatchLiteralInstruction *to_match_literal_instruction(PyObject *pyinstruction) {
    std::shared_ptr<PyObject> pyliteral = {
        PyObject_GetAttrString(pyinstruction, "literal"),
        Py_DecRef,
    };
    if (PyErr_Occurred()) {
        return nullptr;
    }

    auto literal = to_u32string(pyliteral.get());
    if (PyErr_Occurred()) {
        return nullptr;
    } else {
        return new ufpeg::MatchLiteralInstruction(literal);
    }
}

ufpeg::BranchInstruction *to_branch_instruction(PyObject *pyinstruction) {
    std::shared_ptr<PyObject> pysuccess = {
        PyObject_GetAttrString(pyinstruction, "success"),
        Py_DecRef,
    };
    std::shared_ptr<PyObject> pyfailure = {
        PyObject_GetAttrString(pyinstruction, "failure"),
        Py_DecRef,
    };
    if (PyErr_Occurred()) {
        return nullptr;
    }

    auto success = PyLong_AsSize_t(pysuccess.get());
    auto failure = PyLong_AsSize_t(pyfailure.get());
    if (PyErr_Occurred()) {
        return nullptr;
    } else {
        return new ufpeg::BranchInstruction(success, failure);
    }
}

ufpeg::JumpInstruction *to_jump_instruction(PyObject *pyinstruction) {
    std::shared_ptr<PyObject> pypointer = {
        PyObject_GetAttrString(pyinstruction, "pointer"),
        Py_DecRef,
    };
    if (PyErr_Occurred()) {
        return nullptr;
    }

    auto pointer = PyLong_AsSize_t(pypointer.get());
    if (PyErr_Occurred()) {
        return nullptr;
    } else {
        return new ufpeg::JumpInstruction(pointer);
    }
}

ufpeg::PassInstruction *to_pass_instruction(PyObject*) {
    return new ufpeg::PassInstruction();
}

ufpeg::FlipInstruction *to_flip_instruction(PyObject*) {
    return new ufpeg::FlipInstruction();
}

ufpeg::ExpectInstruction *to_expect_instruction(PyObject *pyinstruction) {
    std::shared_ptr<PyObject> pyname = {
        PyObject_GetAttrString(pyinstruction, "name"),
        Py_DecRef,
    };
    if (PyErr_Occurred()) {
        return nullptr;
    }

    auto name = to_u32string(pyname.get());
    if (PyErr_Occurred()) {
        return nullptr;
    } else {
        return new ufpeg::ExpectInstruction(name);
    }
}

const std::map<std::string, std::function<ufpeg::BaseInstruction*(PyObject*)>> converters {
    { "ufpeg.instructions.InvokeInstruction", to_invoke_instruction },
    { "ufpeg.instructions.RevokeInstruction", to_revoke_instruction },
    { "ufpeg.instructions.PrepareInstruction", to_prepare_instruction },
    { "ufpeg.instructions.ConsumeInstruction", to_consume_instruction },
    { "ufpeg.instructions.DiscardInstruction", to_discard_instruction },
    { "ufpeg.instructions.BeginInstruction", to_begin_instruction },
    { "ufpeg.instructions.CommitInstruction", to_commit_instruction },
    { "ufpeg.instructions.AbortInstruction", to_abort_instruction },
    { "ufpeg.instructions.MatchLiteralInstruction", to_match_literal_instruction },
    { "ufpeg.instructions.BranchInstruction", to_branch_instruction },
    { "ufpeg.instructions.JumpInstruction", to_jump_instruction },
    { "ufpeg.instructions.PassInstruction", to_pass_instruction },
    { "ufpeg.instructions.FlipInstruction", to_flip_instruction },
    { "ufpeg.instructions.ExpectInstruction", to_expect_instruction },
};

PyObject *run(PyObject *self, PyObject *args) {
    PyObject *pytext, *pyinstructions;

    if (!PyArg_ParseTuple(args, "UO", &pytext, &pyinstructions)) {
        return nullptr;
    }

    std::u32string text = to_u32string(pytext);
    if (PyErr_Occurred()) {
        return nullptr;
    }

    std::vector<std::shared_ptr<ufpeg::BaseInstruction>> instructions;

    std::shared_ptr<PyObject> iter(PyObject_GetIter(pyinstructions), Py_DecRef);
    if (PyErr_Occurred()) {
        return nullptr;
    }

    while (true) {
        std::shared_ptr<PyObject> pyinstruction(PyIter_Next(iter.get()), Py_DecRef);
        if (PyErr_Occurred()) {
            return nullptr;
        } else if (!pyinstruction) {
            break;
        }

        std::shared_ptr<PyObject> pymodule = {
            PyObject_GetAttrString(pyinstruction.get(), "__module__"),
            Py_DecRef,
        };
        if (PyErr_Occurred()) {
            return nullptr;
        }

        std::shared_ptr<PyObject> pyclass = {
            PyObject_GetAttrString(pyinstruction.get(), "__class__"),
            Py_DecRef,
        };
        if (PyErr_Occurred()) {
            return nullptr;
        }

        std::shared_ptr<PyObject> pyname = {
            PyObject_GetAttrString(pyclass.get(), "__name__"),
            Py_DecRef,
        };
        if (PyErr_Occurred()) {
            return nullptr;
        }

        std::string fullname = PyUnicode_AsUTF8(pymodule.get());
        fullname += '.';
        fullname += PyUnicode_AsUTF8(pyname.get());
        try {
            auto converter = converters.at(fullname);
            auto instruction = converter(pyinstruction.get());
            if (instruction) {
                instructions.emplace_back(instruction);
            } else {
                return nullptr;
            }
        } catch (std::out_of_range&) {
            return PyErr_Format(
                PyExc_TypeError,
                "%R is an unknown instruction",
                pyinstruction.get()
            );
        }
    }

    ufpeg::Vm vm(instructions);

    vm.run(text);

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
