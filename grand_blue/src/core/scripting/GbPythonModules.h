/**
   @file          GbPythonModules.h
   @brief         Contains all Python Modules defined for the Reverie Engine
*/
#ifndef GB_PYTHON_MODULES_H
#define GB_PYTHON_MODULES_H

// External
#include "GbPythonWrapper.h"

// Internal
#include "py_wrappers/GbPyCamera.h"
#include "py_wrappers/GbPyCharacterController.h"
#include "py_wrappers/GbPyComponent.h"
#include "py_wrappers/GbPyCustomEvent.h"
#include "py_wrappers/GbPyEngine.h"
#include "py_wrappers/GbPyInputHandler.h"
#include "py_wrappers/GbPyLight.h"
#include "py_wrappers/GbPyMatrix.h"
#include "py_wrappers/GbPyResourceCache.h"
#include "py_wrappers/GbPyScenario.h"
#include "py_wrappers/GbPyScene.h"
#include "py_wrappers/GbPySceneObject.h"
#include "py_wrappers/GbPyScriptBehavior.h"
#include "py_wrappers/GbPyScriptListener.h"
#include "py_wrappers/GbPyShaderComponent.h"
#include "py_wrappers/GbPyShaderProgram.h"
#include "py_wrappers/GbPySkeletalAnimation.h"
#include "py_wrappers/GbPyTransformComponent.h"
#include "py_wrappers/GbPyVector.h"

namespace Gb {

// Create Reverie Module, wrapping up all binding code
PYBIND11_EMBEDDED_MODULE(reverie, m) {
    PyScriptBehavior::PyInitialize(m);
    PyScriptListener::PyInitialize(m);

    // Define top-level module
    PyScenario::PyInitialize(m);
    PyScene::PyInitialize(m);
    PySceneObject::PyInitialize(m);
    PyCoreEngine::PyInitialize(m);
    PyResourceCache::PyInitialize(m);
    PyInputHandler::PyInitialize(m);
    PyShaderProgram::PyInitialize(m);

    // Define submodule for components
    py::module_ cm = m.def_submodule("components", "Submodule containing all Reverie Scene/Scene Object components");
    PyComponent::PyInitialize(cm);
    PyLight::PyInitialize(cm);
    PyShaderComponent::PyInitialize(cm);
    PyTransformComponent::PyInitialize(cm);

    // Initialize linear algebra types
    PyVector::PyInitialize<real_g, 2>(m, "Vector2");
    PyVector::PyInitialize<real_g, 3>(m, "Vector3");
    PyVector::PyInitialize<real_g, 4>(m, "Vector4");
    PyMatrix::PyInitialize<real_g, 2>(m, "Matrix2x2");
    PyMatrix::PyInitialize<real_g, 3>(m, "Matrix3x3");
    PyMatrix::PyInitialize<real_g, 4>(m, "Matrix4x4");

    // Define submodule for events
    py::module_ eventModule = m.def_submodule("events", "Submodule containing all Reverie event Types");
    PyCustomEvent::PyInitialize(eventModule);
}

}

#endif