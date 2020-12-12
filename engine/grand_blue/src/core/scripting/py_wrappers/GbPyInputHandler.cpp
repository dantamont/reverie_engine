#include "GbPyInputHandler.h"
#include "../../GbCoreEngine.h"
#include "../../input/GbInputHandler.h"

namespace Gb {
void PyInputHandler::PyInitialize(py::module_ & m)
{
    // Initialize Scene class
    py::class_<InputHandler>(m, "InputHandler")
        .def("__repr__", [](const InputHandler& i) {return i.asQString().toStdString(); })
        ;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}