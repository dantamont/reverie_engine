#include "core/scripting/py_wrappers/GPyShaderProgram.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "fortress/json/GJson.h"
#include "core/scripting/GPythonAPI.h"

namespace rev {


void PyShaderProgram::PyInitialize(py::module_ & m)
{
    py::class_<ShaderProgram, std::shared_ptr<ShaderProgram>>(m, "ShaderProgram")
        .def_property_readonly("name",
            [](const ShaderProgram& s) {return s.handle()->getName().c_str(); })
        // FIXME: This no longer works since shaders don't cache uniform values
        //.def_property_readonly("get_uniform",
        //    [](const ShaderProgram& s, const char* uniformName) {
        //        const Uniform* uniformPtr = s.getUniformValue(uniformName);
        //        if (!uniformPtr) {
        //            Logger::Throw("Error, uniform not found");
        //        }
        //        const Uniform& uniform = *uniformPtr;
        //        return uniformToPyObject(uniform);
        //    })
        .def("__str__", [](const ShaderProgram& s) {return GJson::ToString<std::string>(json{s}); })
        .def("__repr__", [](const ShaderProgram& s) {return GJson::ToString<std::string>(json{ s }); })
        ;
}

//void PyShaderProgram::set_uniform(ShaderProgram * s, const QString & uniformName, Matrix2x2g * value)
//{
//    s->setUniforValue(uniformName, value->toRealMatrix());
//}

//void PyShaderProgram::set_uniform(ShaderProgram * s, const QString & uniformName, Matrix3x3g * value)
//{
//    s->setUnifomValue(uniformName, value->toRealMatrix());
//}

//void PyShaderProgram::set_uniform(ShaderProgram * s, const QString & uniformName, Matrix4x4g * value)
//{
//    s->setUnifomValue(uniformName, value->toRealMatrix());
//}

//void PyShaderProgram::set_uniform(ShaderProgram* s, const QString & uniformName, PyObject * value)
//{
//    // Convert to uniform 
//    QVariant variant = PythonAPI::Instance().toQVariant(value);
//    if (!variant.isValid()) {
//        Logger::Throw("type not handled");
//    }
//    s->setUniformalue(uniformName, variant);
//}

//PyObject* PyShaderProgram::uniformToPyObject(const Uniform & uniform)
//{
//    PythonAPI& api = PythonAPI::Instance();
//    PyObject* po;
//    switch (uniform.getType()) {
//    if (uniform.is<int>()) {
//        po = api.toPyLon(uniform.getValue<int>(m_uniforms));
//    }
//    else if (uniform.is<bool>()) {
//        po = api.toPyBool(uniform.getValue<bool>(m_uniforms));
//    }
//    else if (uniform.is<Real_t>()) {
//        po = api.toPyFloat(uniform.getValue<Real_t>(m_uniforms));
//    }
//    else if (uniform.is<Vector2>()) {
//        po = api.toPyTuple(uniform.getValue<Vector2>(m_uniforms).asStdVector());
//    }
//    else if (uniform.is<Vector3>()) {
//        po = api.toPyTuple(uniform.getValue<Vector3>(m_uniforms).asStdVector());
//    }
//    else if (uniform.is<Vector4>()) {
//        po = api.toPyTuple(uniform.getValue<Vector4>(m_uniforms).asStdVector());
//    }
//    else if (uniform.is<Matrix2x2g>()) {
//        po = api.toPyTuple(uniform.getValue<Matrix2x2g>(m_uniforms).asStdVector());
//    }
//    else if (uniform.is<Matrix3x3g>()) {
//        po = api.toPyTuple(uniform.getValue<Matrix3x3g>(m_uniforms).asStdVector());
//    }
//    else if (uniform.is<Matrix4x4g>()) {
//        po = api.toPyTuple(uniform.getValue<Matrix4x4g>(m_uniforms).asStdVector());
//    }
//    else if (uniform.is<std::vector<Real_t>>()) {
//        po = api.toPyTuple(uniform.getValue<std::vector<Real_t>>(m_uniforms));
//    }
//    else if (uniform.is<Vec3List>()) {
//        std::vector<std::vector<Real_t>> vecs;
//        for (const Vector3& vec : uniform.getValue<Vec3List>(m_uniforms)) {
//            Vec::EmplaceBack(vecs, vec.asStdVector());
//        }
//        po = api.toPyTuple(vecs);
//    }
//    default:
//#ifdef DEBUG_MODE
//        QString err = "Error, this uniform to PyObject conversion is not supported";
//        QString typeName = QString::fromStdString(uniform.typeInfo().name());
//        err += ": " + typeName;
//        qDebug() << err;
//        po = nullptr;
//#endif
//    }
//
//    return po;
//}


}