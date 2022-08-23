#include "core/scripting/GPythonScript.h"

#include "fortress/system/path/GFile.h"

#include "core/scripting/GPythonAPI.h"
#include "core/GCoreEngine.h"
#include "core/readers/GFileReader.h"
#include "fortress/json/GJson.h"

#include "core/scripting/GScriptBehavior.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"
#include "core/processes/GProcessManager.h"

namespace rev {


// Python Class Script

PythonScript::PythonScript(CoreEngine * engine) :
    Resource(),
    m_engine(engine)
{
}

PythonScript::PythonScript(CoreEngine * engine, const QString & filepath) :
    LoadableInterface(filepath.toStdString()),
    Resource(),
    m_engine(engine)
{
}


PythonScript::~PythonScript()
{
}

void PythonScript::reload()
{
    initialize();
}

void to_json(json& orJson, const PythonScript& korObject)
{
    ToJson<LoadableInterface>(orJson, korObject);
}

void from_json(const json& korJson, PythonScript& orObject)
{
#ifdef DEBUG_MODE
    GString objStr = GJson::ToString<GString>(korJson);
#endif
    if (korJson.contains(JsonKeys::s_filepath)) {
        // Check for non-null filepath before loading
        GString filePath = korJson[JsonKeys::s_filepath].get_ref<const std::string&>().c_str();
        if (!filePath.isEmpty()) {
            FromJson<LoadableInterface>(korJson, orObject);
        }
    }
}

py::object PythonScript::getPythonSceneObject(const std::shared_ptr<SceneObject>& so) const
{
    // Get scene object from python
    GString sceneObjectName = "so_" + GString::FromNumber(so->id());
    py::object pySceneObject = PythonAPI::Instance().getVariable(sceneObjectName);
    if (!pySceneObject.ptr()) {
        //GString createCode = sceneObjectName +
        //    " = SceneObject(" + "'" +  + "'" + ")\n";
        //PythonAPI::Instance().runCode(createCode);
        //pySceneObject = PythonAPI::Instance().getVariable(sceneObjectName);
        py::module_ revModule = PythonAPI::Instance().reverieModule();
        py::object soClass = revModule.attr("SceneObject");
        pySceneObject = soClass(so->id());
    }

    if (!pySceneObject.ptr()) {
        Logger::Throw("Error, failed to create scene object in python");
    }

    return pySceneObject;
}

void PythonScript::defineInPython()
{
    if (m_contents.isEmpty()) {
        Logger::Throw("Error, no script contents");
    }

    PythonAPI::Instance().runCode(m_contents.toStdString());
//    if (!ran) {
//#ifdef DEBUG_MODE
//        QString stdErr = PythonAPI::Instance().getStdErr();
//        //Logger::Throw("Error, failed to initialize class script in python " + stdErr);
//        Logger::LogError("Error, failed to initialize class script in python " + stdErr);
//#else
//        Logger::LogError("Error, failed to initialize class script in python " + stdErr);
//#endif
//    }
}



// Python Class Script


PythonClassScript::PythonClassScript(CoreEngine* engine, const nlohmann::json& json) :
    PythonScript(engine)
{
    json.get_to(*this);
    initialize();
}

PythonClassScript::PythonClassScript(CoreEngine* engine, const QString& filepath):
    PythonScript(engine, filepath)
{
    initialize();
}

PythonClassScript::~PythonClassScript()
{
}

void PythonClassScript::defineInPython()
{
    PythonScript::defineInPython();
}

py::object PythonClassScript::instantiate(const std::shared_ptr<SceneObject>& sceneObject)
{
    // Get scene object from python
    py::object pySceneObject = getPythonSceneObject(sceneObject);

    if (!pySceneObject.ptr()) {
        Logger::Throw("Error, failed to create scene object in python");
    }
    //auto dictInfo = PythonAPI::Instance().getDictStr(pySceneObject);
    //auto info = PythonAPI::Instance().getInfo(pySceneObject);
    //auto moduleInfo = PythonAPI::Instance().getInfo(PythonAPI::Instance().mainModule());

    // Create argument list to instantiate class
    //PyObject* argList = Py_BuildValue("(O)", pySceneObject.ptr());

    // Instantiate class with a scene object as the argument
    py::object instance = PythonAPI::Instance().instantiate(m_className.toStdString(), pySceneObject);
    //auto instanceInfo = PythonAPI::Instance().getInfo(instance);
    //PythonQtObjectPtr wrappedInstance = py::object(instance);

    return instance;
}

void PythonClassScript::onRemoval(ResourceCache*)
{
}

void to_json(json& orJson, const PythonClassScript& korObject)
{
    ToJson<PythonScript>(orJson, korObject);
    if (korObject.m_sortingLayer) {
        const GString& layerName = korObject.m_engine->scenario()->settings().renderLayers().getLayerNameFromId(korObject.m_sortingLayer);
        orJson["executionLayerName"] = layerName.c_str();
    }
}

void from_json(const json& korJson, PythonClassScript& orObject)
{
    FromJson<PythonScript>(korJson, orObject);

    if (korJson.contains("executionLayerName")) {
        GString name = korJson["executionLayerName"].get_ref<const std::string&>().c_str();
        orObject.m_sortingLayer = orObject.m_engine->processManager()->processQueue().sortingLayers().getLayer(name).id();
    }
    
}

void PythonClassScript::initialize()
{
    static constexpr bool s_removeExtension = false;
    static constexpr bool s_caseInsensitive = false;

    // If there is no sorting layer, set to default
    if (m_sortingLayer == s_invalidSortingLayerValue) {
        m_sortingLayer = m_engine->processManager()->processQueue().sortingLayers().getLayer(SortingLayer::s_defaultSortingLayer).id();
    }

    /// @todo Fix file-loading to work with resource system so file can be found
    GFile pythonClassFile(m_path);
    //m_contents = pythonClassFile.read();
    m_contents = FileReader::GetResourceFileContents(m_path);

    // Get filename without extension
    QString snakeClass = pythonClassFile.getFileName(s_removeExtension, s_caseInsensitive);
    QStringList strs = snakeClass.split("_");
    m_className = "";
    for (QString& s : strs) {
        s = s.replace(0, 1, s[0].toUpper());
        m_className += s;
    }

    // Define class in python
    defineInPython();
}




}