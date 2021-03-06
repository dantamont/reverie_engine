#include "GPythonScript.h"

#include "GPythonAPI.h"
#include "../GCoreEngine.h"
#include "../readers/GFileReader.h"
#include "../readers/GJsonReader.h"

#include "GScriptBehavior.h"
#include "../scene/GSceneObject.h"
#include "../processes/GProcessManager.h"

namespace rev {
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Python Class Script
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PythonScript::PythonScript(CoreEngine * engine) :
    Resource(),
    m_engine(engine)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PythonScript::PythonScript(CoreEngine * engine, const QString & filepath) :
    Loadable(filepath),
    Resource(),
    m_engine(engine)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
PythonScript::~PythonScript()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonScript::reload()
{
    initialize();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue PythonScript::asJson(const SerializationContext& context) const
{
    return Loadable::asJson(context);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonScript::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    QJsonObject object = json.toObject();
#ifdef DEBUG_MODE
    QString objStr = JsonReader::ToString<QString>(object);
#endif
    if (object.contains("filePath")) {
        // Check for non-null filepath before loading
        if (!object["filePath"].toString().isEmpty()) {
            Loadable::loadFromJson(json);
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
py::object PythonScript::getPythonSceneObject(const std::shared_ptr<SceneObject>& so) const
{
    // Get scene object from python
    GString sceneObjectName = "so_" + GString::FromNumber(so->id());
    py::object pySceneObject = PythonAPI::get()->getVariable(sceneObjectName);
    if (!pySceneObject.ptr()) {
        //GString createCode = sceneObjectName +
        //    " = SceneObject(" + "'" +  + "'" + ")\n";
        //PythonAPI::get()->runCode(createCode);
        //pySceneObject = PythonAPI::get()->getVariable(sceneObjectName);
        py::module_ revModule = PythonAPI::get()->reverieModule();
        py::object soClass = revModule.attr("SceneObject");
        pySceneObject = soClass(so->id());
    }

    if (!pySceneObject.ptr()) {
        throw("Error, failed to create scene object in python");
    }

    return pySceneObject;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonScript::defineInPython()
{
    if (m_contents.isEmpty()) {
        throw("Error, no script contents");
    }

    PythonAPI::get()->runCode(m_contents);
//    if (!ran) {
//#ifdef DEBUG_MODE
//        QString stdErr = PythonAPI::get()->getStdErr();
//        //throw("Error, failed to initialize class script in python " + stdErr);
//        logError("Error, failed to initialize class script in python " + stdErr);
//#else
//        logError("Error, failed to initialize class script in python " + stdErr);
//#endif
//    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Python Class Script
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PythonClassScript::PythonClassScript(CoreEngine* engine, const QJsonValue& json) :
    PythonScript(engine)
{
    loadFromJson(json);
    initialize();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PythonClassScript::PythonClassScript(CoreEngine* engine, const QString& filepath):
    PythonScript(engine, filepath)
{
    initialize();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PythonClassScript::~PythonClassScript()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonClassScript::defineInPython()
{
    PythonScript::defineInPython();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
py::object PythonClassScript::instantiate(const std::shared_ptr<SceneObject>& sceneObject)
{
    // Get scene object from python
    py::object pySceneObject = getPythonSceneObject(sceneObject);

    if (!pySceneObject.ptr()) {
        throw("Error, failed to create scene object in python");
    }
    //auto dictInfo = PythonAPI::get()->getDictStr(pySceneObject);
    //auto info = PythonAPI::get()->getInfo(pySceneObject);
    //auto moduleInfo = PythonAPI::get()->getInfo(PythonAPI::get()->mainModule());

    // Create argument list to instantiate class
    //PyObject* argList = Py_BuildValue("(O)", pySceneObject.ptr());

    // Instantiate class with a scene object as the argument
    py::object instance = PythonAPI::get()->instantiate(m_className, pySceneObject);
    //auto instanceInfo = PythonAPI::get()->getInfo(instance);
    //PythonQtObjectPtr wrappedInstance = py::object(instance);

    return instance;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue PythonClassScript::asJson(const SerializationContext& context) const
{
    QJsonObject jsonObject = PythonScript::asJson(context).toObject();
    if (m_sortingLayer) {
        jsonObject.insert("executionLayerName", m_sortingLayer->getName().c_str());
    }

    return jsonObject;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonClassScript::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    PythonScript::loadFromJson(json);
    const QJsonObject& object = json.toObject();

    if (object.contains("executionLayerName")) {
        QString name = object["executionLayerName"].toString();
        m_sortingLayer = m_engine->processManager()->sortingLayers().getLayer(name);
    }
    
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonClassScript::initialize()
{
    // If there is no sorting layer, set to default
    if (!m_sortingLayer) {
        m_sortingLayer = m_engine->processManager()->sortingLayers().getLayer(DEFAULT_SORTING_LAYER);
    }

    //m_contents = PythonQt::self()->parseFile(m_path); // Was raising errors
    m_contents = FileReader::getContentsAsString(m_path.c_str());

    // Get filename without extension
    QString snakeClass = FileReader::PathToName(m_path.c_str()).split(".")[0];
    QStringList strs = snakeClass.split("_");
    m_className = "";
    for (QString& s : strs) {
        s = s.replace(0, 1, s[0].toUpper());
        m_className += s;
    }

    // Define class in python
    defineInPython();
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////
}