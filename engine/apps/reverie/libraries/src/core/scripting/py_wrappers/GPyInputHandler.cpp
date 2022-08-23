#include "core/scripting/py_wrappers/GPyInputHandler.h"
#include "core/GCoreEngine.h"
#include "core/layer/view/widgets/graphics/GInputHandler.h"

namespace rev {
void PyInputHandler::PyInitialize(py::module_ & m)
{
    // Mouse handler
    py::class_<MouseHandler>(m, "MouseHandler")
        .def("__repr__", [](const MouseHandler& m) {return "Mouse Handler"; })
        .def("was_pressed", [](MouseHandler& m, int key) {return m.wasPressed(Qt::MouseButton(key)); })
        .def("was_released", [](MouseHandler& m, int key) {return m.wasReleased(Qt::MouseButton(key)); })
        .def("was_double_clicked", [](MouseHandler& m, int key) {return m.wasDoubleClicked(Qt::MouseButton(key)); })
        .def("is_held", [](MouseHandler& m, int key) {return m.isHeld(Qt::MouseButton(key)); })
        .def("moved", &MouseHandler::wasMoved)
        .def("scrolled", &MouseHandler::wasScrolled)
        .def("mouse_delta", &MouseHandler::mouseDelta)
        .def("scroll_delta", &MouseHandler::scrollDelta)
        .def("screen_pos", &MouseHandler::normalizeMousePosition)
        ;

    // Key handler
    py::class_<KeyHandler>(m, "KeyHandler")
        .def("__repr__", [](const KeyHandler& k) {return "Key Handler"; })
        .def("was_pressed", [](KeyHandler& k, int key) {return k.wasPressed(Qt::Key(key)); })
        .def("was_pressed", [](KeyHandler& k, const char* key) {return k.wasPressed(key); })
        .def("was_released", [](KeyHandler& k, int key) {return k.wasReleased(Qt::Key(key)); })
        .def("was_released", [](KeyHandler& k, const char* key) {return k.wasReleased(key); })
        .def("was_double_clicked", [](KeyHandler& k, int key) {return k.wasDoubleClicked(Qt::Key(key)); })
        .def("was_double_clicked", [](KeyHandler& k, const char* key) {return k.wasDoubleClicked(key); })
        .def("is_held", [](KeyHandler& k, int key) {return k.isHeld(Qt::Key(key)); })
        .def("is_held", [](KeyHandler& k, const char* key) {return k.isHeld(key); })
        ;

    py::class_<InputHandler>(m, "InputHandler")
        .def("__repr__", [](const InputHandler& i) {return "Input Handler"; })
        //.def_property_readonly("key_handler", py::overload_cast<>(&InputHandler::keyHandler, py::const_))
        .def_property_readonly("key_handler", py::overload_cast<>(&InputHandler::keyHandler))
        //.def_property_readonly("mouse_handler", py::overload_cast<>(&InputHandler::mouseHandler, py::const_))
        .def_property_readonly("mouse_handler", py::overload_cast<>(&InputHandler::mouseHandler))
        ;

}


}