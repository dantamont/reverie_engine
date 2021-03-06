//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Wrap up Python inclusion to avoid conflicts with QT "slots" keyword
// See: https://stackoverflow.com/questions/23068700/-python3-in-qt-5
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef GB_PYTHON_WRAPPER_H
#define GB_PYTHON_WRAPPER_H

#pragma push_macro("slots")
#undef slots
//#define PY_SSIZE_T_CLEAN  /* Make "s#" use Py_ssize_t rather than int. */
//#include <Python.h>
#include <pybind11/embed.h> // everything needed for embedding (needs to be included before Qt stuff pollutes slots)
#include <pybind11/pybind11.h> 
#pragma pop_macro("slots")

namespace py = pybind11;


#endif