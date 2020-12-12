#include "GbPyShaderProgram.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../../rendering/shaders/GbShaders.h"
#include "../../readers/GbJsonReader.h"
#include "../../scripting/GbPythonAPI.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyShaderProgram::PyInitialize(py::module_ & m)
{
    py::class_<ShaderProgram, std::shared_ptr<ShaderProgram>>(m, "ShaderProgram")
        .def_property_readonly("name",
            [](const ShaderProgram& s) {return s.getName().c_str(); })
        .def_property_readonly("get_uniform",
            [](const ShaderProgram& s, const char* uniformName) {
                const Uniform* uniformPtr = s.getUniformValue(uniformName);
                if (!uniformPtr) {
                    throw("Error, uniform not found");
                }
                const Uniform& uniform = *uniformPtr;
                return uniformToPyObject(uniform);
            })
        .def("__str__", [](const ShaderProgram& s) {return JsonReader::ToGString(s.asJson()).toStdString(); })
        .def("__repr__", [](const ShaderProgram& s) {return s.asQString().toStdString(); })
        ;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//void PyShaderProgram::set_uniform(ShaderProgram * s, const QString & uniformName, Matrix2x2g * value)
//{
//    s->setUniformValue(uniformName, value->toRealMatrix());
//}
///////////////////////////////////////////////////////////////////////////////////////////////
//void PyShaderProgram::set_uniform(ShaderProgram * s, const QString & uniformName, Matrix3x3g * value)
//{
//    s->setUniformValue(uniformName, value->toRealMatrix());
//}
///////////////////////////////////////////////////////////////////////////////////////////////
//void PyShaderProgram::set_uniform(ShaderProgram * s, const QString & uniformName, Matrix4x4g * value)
//{
//    s->setUniformValue(uniformName, value->toRealMatrix());
//}
/////////////////////////////////////////////////////////////////////////////////////////////////
//void PyShaderProgram::set_uniform(ShaderProgram* s, const QString & uniformName, PyObject * value)
//{
//    // Convert to uniform 
//    QVariant variant = PythonAPI::get()->toQVariant(value);
//    if (!variant.isValid()) {
//        throw("type not handled");
//    }
//    s->setUniformValue(uniformName, variant);
//}
/////////////////////////////////////////////////////////////////////////////////////////////
PyObject* PyShaderProgram::uniformToPyObject(const Uniform & uniform)
{
    PythonAPI* api = PythonAPI::get();
    PyObject* po;
    if (uniform.is<int>()) {
        po = api->toPyLon(uniform.get<int>());
    }
    else if (uniform.is<bool>()) {
        po = api->toPyBool(uniform.get<bool>());
    }
    else if (uniform.is<real_g>()) {
        po = api->toPyFloat(uniform.get<real_g>());
    }
    else if (uniform.is<Vector2>()) {
        po = api->toPyTuple(uniform.get<Vector2>().asStdVector());
    }
    else if (uniform.is<Vector3>()) {
        po = api->toPyTuple(uniform.get<Vector3>().asStdVector());
    }
    else if (uniform.is<Vector4>()) {
        po = api->toPyTuple(uniform.get<Vector4>().asStdVector());
    }
    else if (uniform.is<Matrix2x2g>()) {
        po = api->toPyTuple(uniform.get<Matrix2x2g>().asStdVector());
    }
    else if (uniform.is<Matrix3x3g>()) {
        po = api->toPyTuple(uniform.get<Matrix3x3g>().asStdVector());
    }
    else if (uniform.is<Matrix4x4g>()) {
        po = api->toPyTuple(uniform.get<Matrix4x4g>().asStdVector());
    }
    else if (uniform.is<std::vector<real_g>>()) {
        po = api->toPyTuple(uniform.get<std::vector<real_g>>());
    }
    else if (uniform.is<Vec3List>()) {
        std::vector<std::vector<real_g>> vecs;
        for (const Vector3& vec : uniform.get<Vec3List>()) {
            Vec::EmplaceBack(vecs, vec.asStdVector());
        }
        po = api->toPyTuple(vecs);
    }
    else {
#ifdef DEBUG_MODE
        QString err = "Error, this uniform to PyObject conversion is not supported";
        QString typeName = QString::fromStdString(uniform.typeInfo().name());
        err += ": " + typeName;
        qDebug() << err;
        po = nullptr;
#endif
    }

    return po;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}