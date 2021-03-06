/**
   @file          GbPythonModules.h
   @brief         Contains all Python Modules defined for the Reverie Engine
*/
#ifndef GB_PYTHON_MODULES_H
#define GB_PYTHON_MODULES_H

// External
#include "GPythonWrapper.h"

// Internal
#include "py_wrappers/GPyAudioComponent.h"
#include "py_wrappers/GPyCamera.h"
#include "py_wrappers/GPyCharacterController.h"
#include "py_wrappers/GPyComponent.h"
#include "py_wrappers/GPyCustomEvent.h"
#include "py_wrappers/GPyEngine.h"
#include "py_wrappers/GPyInputHandler.h"
#include "py_wrappers/GPyLight.h"
#include "py_wrappers/GPyMatrix.h"
#include "py_wrappers/GPyResourceCache.h"
#include "py_wrappers/GPyRotations.h"
#include "py_wrappers/GPyScenario.h"
#include "py_wrappers/GPyScene.h"
#include "py_wrappers/GPySceneObject.h"
#include "py_wrappers/GPyScriptBehavior.h"
#include "py_wrappers/GPyScriptListener.h"
#include "py_wrappers/GPyShaderComponent.h"
#include "py_wrappers/GPyShaderProgram.h"
#include "py_wrappers/GPySkeletalAnimation.h"
#include "py_wrappers/GPyTransformComponent.h"
#include "py_wrappers/GPyVector.h"

namespace rev {

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

    // Initialize linear algebra types (order here matters)
    PyVector::PyInitialize<real_g, 2>(m, "Vector2");
    PyVector::PyInitialize<real_g, 3>(m, "Vector3");
    PyVector::PyInitialize<real_g, 4>(m, "Vector4");
    PyMatrix::PyInitialize<real_g, 2>(m, "Matrix2x2");
    PyMatrix::PyInitialize<real_g, 3>(m, "Matrix3x3");
    PyMatrix::PyInitialize<real_g, 4>(m, "Matrix4x4");
    PyMatrix::PyInitialize<double, 2>(m, "Matrix2x2d");
    PyMatrix::PyInitialize<double, 3>(m, "Matrix3x3d");
    PyMatrix::PyInitialize<double, 4>(m, "Matrix4x4d");
    PyRotations::PyInitialize(m);

    // Define submodule for components
    py::module_ cm = m.def_submodule("components", "Submodule containing all Reverie Scene/Scene Object components");
    PyComponent::PyInitialize(cm);
    PyAudioSourceComponent::PyInitialize(cm);
    PyCamera::PyInitialize(cm);
    PyLight::PyInitialize(cm);
    PyCharController::PyInitialize(cm);
    PySkeletalAnimation::PyInitialize(cm);
    PyShaderComponent::PyInitialize(cm);
    PyTransformComponent::PyInitialize(cm);

    // Define submodule for events
    py::module_ eventModule = m.def_submodule("events", "Submodule containing all Reverie event Types");
    PyCustomEvent::PyInitialize(eventModule);
}

}

#endif