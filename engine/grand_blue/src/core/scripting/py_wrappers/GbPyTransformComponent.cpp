#include "GbPyTransformComponent.h"
#include "../../components/GbTransformComponent.h"
#include "../../readers/GbJsonReader.h"

namespace Gb {
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyTransformComponent::PyInitialize(py::module_ & m)
{
    // Initialize Engine class
    py::class_<TransformComponent, Component>(m, "TransformComponent")
        .def("__repr__", [](const TransformComponent* t) {
        return JsonReader::ToQString(t->asJson()).toStdString(); }
        )
        ;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
}