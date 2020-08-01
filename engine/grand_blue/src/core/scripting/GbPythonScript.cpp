#include "GbPythonScript.h"

#include "../../third_party/pythonqt/PythonQtConversion.h"
#include "GbPythonAPI.h"
#include "../GbCoreEngine.h"
#include "../readers/GbFileReader.h"
#include "../readers/GbJsonReader.h"

#include "../scripting/GbPyWrappers.h"
#include "../scene/GbSceneObject.h"
#include "../processes/GbProcessManager.h"

namespace Gb {
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PythonBehavior::PythonBehavior()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonBehavior::initialize()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonBehavior::update(unsigned long deltaMs)
{
    Q_UNUSED(deltaMs);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonBehavior::fixedUpdate(unsigned long deltaMs)
{
    Q_UNUSED(deltaMs);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonBehavior::onSuccess()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonBehavior::onFail()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonBehavior::onAbort()
{
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Behavior Wrapper
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PythonBehaviorWrapper::PythonBehaviorWrapper():
    QObject(),
    Object()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PythonBehavior* PythonBehaviorWrapper::new_PythonBehavior() { 
    return new PythonBehavior();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonBehaviorWrapper::initialize(PythonBehavior* o)
{
#ifdef DEBUG_MODE
    logInfo("PYTHON:: Python behavior wrapper is initialized");
#endif
    o->initialize();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonBehaviorWrapper::update(PythonBehavior* o, unsigned long deltaMs)
{
//#ifdef DEBUG_MODE
//    logInfo("PYTHON:: Python behavior is updating");
//#endif
    o->update(deltaMs);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonBehaviorWrapper::fixed_update(PythonBehavior* o, unsigned long deltaMs)
{
//#ifdef DEBUG_MODE
//    logInfo("PYTHON:: Python behavior is fixed updating");
//#endif
    o->fixedUpdate(deltaMs);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonBehaviorWrapper::on_success(PythonBehavior* o)
{
#ifdef DEBUG_MODE
    logInfo("PYTHON:: Python behavior succeeded");
#endif
    o->onSuccess();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonBehaviorWrapper::on_fail(PythonBehavior* o)
{
#ifdef DEBUG_MODE
    logInfo("PYTHON:: Python behavior failed");
#endif
    o->onFail();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonBehaviorWrapper::on_abort(PythonBehavior* o)
{
#ifdef DEBUG_MODE
    logInfo("PYTHON::Python behavior aborted");
#endif
    o->onAbort();
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Python Class Script
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PythonScript::PythonScript(CoreEngine * engine) :
    Resource(kPythonScript),
    m_engine(engine)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PythonScript::PythonScript(CoreEngine * engine, const QString & filepath) :
    Loadable(filepath),
    Resource(kPythonScript),
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
QJsonValue PythonScript::asJson() const
{
    return Loadable::asJson();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonScript::loadFromJson(const QJsonValue & json)
{
    QJsonObject object = json.toObject();
#ifdef DEBUG_MODE
    QString objStr = JsonReader::ToQString(object);
#endif
    if (object.contains("filePath")) {
        // Check for non-null filepath before loading
        if (!object["filePath"].toString().isEmpty()) {
            Loadable::loadFromJson(json);
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject* PythonScript::getPythonSceneObject(const std::shared_ptr<SceneObject>& so) const
{
    // Get scene object from python
    QString sceneObjectName = so->getUuid().createUniqueName("so_");
    PyObject* pySceneObject = PythonAPI::get()->getVariable(sceneObjectName);
    if (!pySceneObject) {
        QString createCode = sceneObjectName +
            " = SceneObject(" + "'" + so->getUuid().asString() + "'" + ")\n";
        PythonAPI::get()->runCode(createCode);
        pySceneObject = PythonAPI::get()->getVariable(sceneObjectName);
    }

    if (!pySceneObject) {
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

    bool ran = PythonAPI::get()->runCode(m_contents);
    if (!ran) {
#ifdef DEBUG_MODE
        QString stdErr = PythonAPI::get()->getStdErr();
        throw("Error, failed to initialize class script in python " + stdErr);
#endif
    }
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
PythonQtObjectPtr PythonClassScript::instantiate(const std::shared_ptr<SceneObject>& sceneObject)
{
    // Get scene object from python
    PyObject* pySceneObject = getPythonSceneObject(sceneObject);

    if (!pySceneObject) {
        throw("Error, failed to create scene object in python");
    }
    //auto dictInfo = PythonAPI::get()->getDictStr(pySceneObject);
    //auto info = PythonAPI::get()->getInfo(pySceneObject);
    //auto moduleInfo = PythonAPI::get()->getInfo(PythonAPI::get()->mainModule());

    // Create argument list to instantiate class
    PyObject* argList = Py_BuildValue("(O)", pySceneObject);

    // Instantiate class with a scene object as the argument
    PyObject* instance = PythonAPI::get()->instantiate(m_className, argList);
    //auto instanceInfo = PythonAPI::get()->getInfo(instance);
    PythonQtObjectPtr wrappedInstance = PythonQtObjectPtr(instance);

    return wrappedInstance;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue PythonClassScript::asJson() const
{
    QJsonObject jsonObject = PythonScript::asJson().toObject();
    if (m_sortingLayer) {
        jsonObject.insert("executionLayerName", m_sortingLayer->getName());
    }

    return jsonObject;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonClassScript::loadFromJson(const QJsonValue & json)
{
    PythonScript::loadFromJson(json);
    const QJsonObject& object = json.toObject();

    if (object.contains("executionLayerName")) {
        QString name = object["executionLayerName"].toString();
        m_sortingLayer = m_engine->processManager()->sortingLayers().at(name);
    }
    
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonClassScript::initialize()
{
    // If there is no sorting layer, set to default
    if (!m_sortingLayer) {
        m_sortingLayer = m_engine->processManager()->sortingLayers().at("default");
    }

    //m_contents = PythonQt::self()->parseFile(m_path); // Was raising errors
    m_contents = FileReader::getContentsAsString(m_path);

    // Get filename without extension
    QString snakeClass = FileReader::pathToName(m_path).split(".")[0];
    QStringList strs = snakeClass.split("_");
    m_className = "";
    for (QString& s : strs) {
        s = s.replace(0, 1, s[0].toUpper());
        m_className += s;
    }

    // Define class in python
    defineInPython();
}


//
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PythonListenerScript
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PythonListenerScript::PythonListenerScript(CoreEngine * engine, const QJsonValue& json) :
//    PythonScript(engine)
//{
//    loadFromJson(json);
//    initialize();
//}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PythonListenerScript::PythonListenerScript(CoreEngine * engine, const QString & filepath) :
//    PythonScript(engine, filepath)
//{
//    initialize();
//}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PythonListenerScript::~PythonListenerScript()
//{
//}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PythonQtObjectPtr PythonListenerScript::instantiate(const std::shared_ptr<SceneObject>& sceneObject)
//{
//    // Get scene object from python
//    PyObject* pySceneObject = getPythonSceneObject(sceneObject);
//
//    // Create argument list to instantiate class
//    PyObject* argList = Py_BuildValue("(O)", pySceneObject);
//
//    // Instantiate class with a scene object as the argument
//    PyObject* instance = PythonAPI::get()->instantiate(m_className, argList);
//    PythonQtObjectPtr wrappedInstance = PythonQtObjectPtr(instance);
//
//    return wrappedInstance;
//}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//QJsonValue PythonListenerScript::asJson() const
//{
//    QJsonObject jsonObject = PythonScript::asJson().toObject();
//
//    return jsonObject;
//}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void PythonListenerScript::loadFromJson(const QJsonValue & json)
//{
//    PythonScript::loadFromJson(json);
//    //const QJsonObject& object = json.toObject();
//}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void PythonListenerScript::initialize()
//{
//    m_contents = FileReader::getContentsAsString(m_path);
//
//    // Get filename without extension
//    // Assumes a unique function name
//    // TODO: Loosen this assumption, i.e., auto-generate unique function name
//    m_className = FileReader::pathToName(m_path).split(".")[0];
//
//    // Define class in python
//    defineInPython();
//}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void PythonListenerScript::defineInPython()
//{
//    PythonScript::defineInPython();
//}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}