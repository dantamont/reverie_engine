#include "core/scripting/py_wrappers/GPyCharacterController.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/GCoreEngine.h"

#include "core/physics/GCharacterController.h"
#include <core/components/GCharControlComponent.h>
#include <core/components/GRigidBodyComponent.h>
#include "fortress/json/GJson.h"

namespace rev {


void PyCharController::PyInitialize(py::module_ & m)
{
    py::class_<CharControlComponent, Component, std::unique_ptr<CharControlComponent, py::nodelete>>(m, "CharacterController")
        .def(py::init<>(&PyComponent::Create<CharControlComponent>))
        .def("__str__", [](const CharControlComponent& c) {GJson::ToString<std::string>(json{c}); })
        .def("__repr__", [](const CharControlComponent& c) {GJson::ToString<std::string>(json{ c }); })
        .def("move", [](CharControlComponent& c, const Vector3& disp) {c.move(disp); })
        .def_property("radius", 
            [](CharControlComponent& c) {
            if (c.controller()->getType() == ControllerDescription::ControllerType::kCapsule) {
                return dynamic_cast<CapsuleController*>(c.controller())->getRadius();
            }
            else {
                return -1.0f;
            }
        },
            [](CharControlComponent& c, float rad) {
            if (c.controller()->getType() == ControllerDescription::ControllerType::kCapsule) {
                dynamic_cast<CapsuleController*>(c.controller())->setRadius(rad);
            }
        })
        .def_property("height",
            [](CharControlComponent& c) {
            if (c.controller()->getType() == ControllerDescription::ControllerType::kCapsule) {
                return dynamic_cast<CapsuleController*>(c.controller())->getHeight();
            }
            else {
                return dynamic_cast<BoxController*>(c.controller())->getHalfHeight() * 2.0f;
            }
        },
            [](CharControlComponent& c, float height) {
            if (c.controller()->getType() == ControllerDescription::ControllerType::kCapsule) {
                bool done = dynamic_cast<CapsuleController*>(c.controller())->setHeight(height);
                return done;
            }
            else {
                return dynamic_cast<BoxController*>(c.controller())->setHalfHeight(height / 2.0f);
            }
        })
        .def_property("height_offset",
            [](CharControlComponent& c) {
                return c.controller()->heightOffset();
            },
            [](CharControlComponent& c, float offset) {
                c.controller()->setHeightOffset(offset);
        })
        .def_property("initial_position",
            [](CharControlComponent& c) {
                return c.controller()->getInitialPosition();
            },
            [](CharControlComponent& c, const Vector3& pos) {
                c.controller()->setInitialPosition(pos);
        })
        ;
}


}