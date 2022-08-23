#include "core/scripting/GPythonAPI.h"
#include <QDir>

#include "fortress/json/GJson.h"
#include "fortress/containers/GContainerExtensions.h"
#include "fortress/system/memory/GPointerTypes.h"
#include "fortress/system/path/GDir.h"

namespace rev{

QString PythonAPI::GetScriptDir()
{
    /// @todo Fix this, should not use current path. Is unreliable
    QString dir = GDir::CurrentWorkingDir().c_str();
    //dir = QGuiApplication::applicationDirPath();
    QString scriptPath = QDir::cleanPath(dir + QDir::separator() + "resources/scripts");
    return scriptPath;
}

rev::PythonAPI::PythonAPI(const GString& programName):
    m_outCatcher(nullptr),
    m_errCatcher(nullptr)
{
    initialize(programName);
}

rev::PythonAPI::~PythonAPI()
{
}

py::module_ PythonAPI::importModule(const GString & packageName) const
{
    return py::module_::import(packageName.c_str());
}

void PythonAPI::runCode(const GString & code) const
{
    try {
        py::exec(code.c_str());
    }
    catch (py::error_already_set& err) {
        // Failing because need from reverie import SceneObject
        const char* errStr = err.what();
#ifdef DEBUG_MODE
        Logger::LogError(errStr);
        Logger::Throw(err);
#else
        Logger::LogError(errStr);
#endif
    }
}

void PythonAPI::runCode(const GString & code, py::object& scope) const
{
    try {
        py::exec(code.c_str(), scope);
    }
    catch (py::error_already_set& err) {
        // Failing because need from reverie import SceneObject
        const char* errStr = err.what();
        Logger::Throw(err);
    }
}

void PythonAPI::runFile(const GString & filepath) const
{
    py::eval_file(filepath.c_str());
    /// \see https://stackoverflow.com/questions/3654652/why-does-the-python-c-api-crash-on-pyrun-simplefile
    //PyObject *obj = Py_BuildValue("s", filepath.c_str());
    //FILE* file = _Py_fopen_obj(obj, "r+");
    //int ran = PyRun_SimpleFile(file, filepath.c_str());
    //if (ran < 0) {
    //    printAndClearErrors();
    //    return false;
    //}
    //Py_DECREF(obj);
    //return true;
}

//QVariant PythonAPI::toQVariant(PyObject * val, int type)
//{
//    QVariant variant = PythonQtConv::PyObjToQVariant(val, type);
//#ifdef DEBUG_MODE
//    if (!variant.isValid()) {
//        int type = variant.userType();
//        QString typeStr = QVariant::typeToName(type);
//        Logger::LogError("Error, QVariant of type " + typeStr + " is invalid.");
//    }
//#endif
//    return variant;
//}

py::object PythonAPI::getClass(const GString & className) const
{
    py::object class_ = getVariable(className);
    if (!class_) {
        Logger::Throw("Error, no class found");
    }
    if (!isClass(class_)) {
        Logger::Throw("Error, object is not a class");
    }
    return class_;
}

bool PythonAPI::isClass(const py::object& cls) const
{
    if (cls) {
        return PyType_Check(cls.ptr());
    }
    return false;
}

GString PythonAPI::getClassName(const py::object& o) const
{
    return GString(o.ptr()->ob_type->tp_name);
}

py::object PythonAPI::instantiate(const GString & className, const py::object& args) const
{
    // Call the class object.
    /// \see https://stackoverflow.com/questions/54288669/instantiate-wrapper-class-from-c
    py::object cls = getClass(className);
    //PyObject* obj = PyObject_CallObject(cls.ptr(), argList);
    py::object obj;
    try {
        obj = cls(args);
    }
    catch(py::error_already_set& err){

        const char* errorStr = err.what();
        //GString argDictInfo = getDictStr(argList);
        //std::vector<GString> argInfo = getInfo(argList);
#ifdef DEBUG_MODE
        //GString errStr = "Bad call to " + className + " constructor, with args" + argDictInfo;
        Logger::Throw(errorStr);
#else
        Logger::LogError("errorStr");
#endif
    }

    // Release the argument list.
    //Py_XDECREF(argList);

    // Release the class string
    // Removed, done by py::object
    //Py_DECREF(cls);

    return obj;
}

bool PythonAPI::isSubclass(const GString & derivedName, const GString & clsName) const
{
    py::object derived = getClass(derivedName);
    py::object cls = getClass(clsName);
    return isSubclass(derived, cls);
}

bool PythonAPI::isSubclass(const py::object& derived, const py::object& cls) const
{
    // Determine whether or not derived is a subclass of cls
    bool isSub = false;
    if (isClass(derived) && isClass(cls)) {
        isSub = PyObject_IsSubclass(derived.ptr(), cls.ptr());
    }
    else {
        GString errStr = "isSubclass:: Error, given objects are not classes";
        Logger::LogError(errStr);
#ifdef DEBUG_MODE
        Logger::Throw(errStr);
#endif
    }

    // REMOVED, done by py::object
    // Decrement class strings
    //Py_DECREF(derived);
    //Py_DECREF(cls);

    return isSub;
}

std::vector<GString> PythonAPI::getInfo(PyObject * o) const
{
    PyObject* info = PyObject_Dir(o);
    printAndClearErrors();
    return toVec<GString>(info);
}

GString PythonAPI::getDictStr(PyObject* o) const
{
    return cType<GString>(getDict(o));
}

PyObject * PythonAPI::getDict(PyObject * o) const
{
    return PyObject_GenericGetDict(o, NULL);
}

PyObject * PythonAPI::getFunction(PyObject * o, const GString & attrName) const
{
    PyObject* callable = getAttribute(o, attrName);
    if (isCallableAttribute(callable)) {
        return callable;
    }
    else {
        // Print error if occurred
        printAndClearErrors();
#ifdef DEBUG_MODE
        Logger::LogError("Attribute is not callable");
#endif
        Py_XDECREF(callable);
    }
    return nullptr;
}

py::object PythonAPI::getVariable(const GString & variableName) const
{
    try {
        return m_main.attr(variableName.c_str());
    }
    catch (py::error_already_set& err) {
        Q_UNUSED(err);
        return py::object();
    }
}

PyObject * PythonAPI::getAttribute(PyObject * o, const GString & attrName) const
{
    // Same as o.attr_name, so need to DECREF when done with attr
    if (!o) {
        //QString err = getStdErr();
        GString errStr = "Error, python object is null: ";// +err;
        Logger::LogError(errStr);
#ifdef DEBUG_MODE
        Logger::Throw(errStr);
#endif
    }
    PyObject* attr = PyObject_GetAttrString(o, attrName.c_str());
    if (attr == NULL) {
        printAndClearErrors();
        //Logger::Throw("error, no attribute obtained");
        return nullptr;
    }
    else {
        return attr;
    }
}

PyObject * PythonAPI::call(const GString & objectName, const GString & methodName, PyObject * args) const
{
    // Get object from object name
    py::object o = getVariable(objectName);

    // Get function name and call
    PyObject* pFunc = getFunction(o.ptr(), methodName);
    PyObject* result = call(pFunc, args);

    // Not needed with py::object
    //Py_DECREF(o);
    return result;
}

PyObject * PythonAPI::call(PyObject * o, const GString & methodName, PyObject * args) const
{
    // Get function name and call
    PyObject* pFunc = getFunction(o, methodName);
    PyObject* result = call(pFunc, args);
    //std::string str = methodName.toStdString();
    //PyObject* result = PyObject_CallMethod(o, str.c_str());
    return result;
}

PyObject * PythonAPI::call(PyObject * o) const
{
    return call(o, NULL);
}

PyObject * PythonAPI::call(PyObject * o, PyObject * args) const
{
    PyObject* output = nullptr;
    if (isCallableAttribute(o) && (isTuple(args) || args == NULL)) {
        output = PyObject_CallObject(o, args);
        if (output == NULL) {
#ifdef DEBUG_MODE
            Logger::LogError("Call failed");
#endif  
        }
    }
    else {
        // Print error if occurred
        printAndClearErrors();
#ifdef DEBUG_MODE
        Logger::LogError("Cannot find callable, or args are not a tuple");
#endif
    }

    // Decrement count for function, ignoring if it is null
    Py_XDECREF(o);

    return output;
}

bool PythonAPI::isCallableAttribute(PyObject * o) const
{
    return o && PyCallable_Check(o);
}

PyObject * PythonAPI::toPyTuple(size_t len) const
{
    return PyTuple_New(len);
}

PyObject * PythonAPI::toPyTuple(const std::vector<int>& vec) const
{
    PyObject* pTuple = toPyTuple(vec.size());
    PyObject* pValue;
    for (unsigned int i = 0; i < vec.size(); i++) {
        pValue = toPyLon(vec[i]);
        if (!pValue) {
            // If invalid conversion to long, raise error
#ifdef DEBUG_MODE
            Logger::LogError("Cannot convert argument for tuple");
#endif
        }
        // Steals reference, so no need to DECREF
        PyTuple_SetItem(pTuple, i, pValue);
    }
    return pTuple;
}

PyObject * PythonAPI::toPyTuple(const std::vector<double>& vec) const
{
    PyObject* pTuple = toPyTuple(vec.size());
    PyObject* pValue;
    for (unsigned int i = 0; i < vec.size(); i++) {
        pValue = toPyFloat(vec[i]);
        if (!pValue) {
            // If invalid conversion to long, raise error
#ifdef DEBUG_MODE
            Logger::LogError("Cannot convert argument for tuple");
#endif
        }
        // Steals reference, so no need to DECREF
        PyTuple_SetItem(pTuple, i, pValue);
    }
    return pTuple;
}

PyObject * PythonAPI::toPyTuple(const std::vector<std::vector<double>>& vec) const
{
    PyObject* pTuple = toPyTuple(vec.size());
    PyObject* pValue;
    for (unsigned int i = 0; i < vec.size(); i++) {
        pValue = toPyTuple(vec[i]);
        if (!pValue) {
            // If invalid conversion to vector, raise error
#ifdef DEBUG_MODE
            Logger::LogError("Cannot convert argument for tuple");
#endif
        }
        // Steals reference, so no need to DECREF
        PyTuple_SetItem(pTuple, i, pValue);
    }
    return pTuple;
}

PyObject * PythonAPI::toPyTuple(const std::vector<std::vector<float>>& vec) const
{
    PyObject* pTuple = toPyTuple(vec.size());
    PyObject* pValue;
    for (unsigned int i = 0; i < vec.size(); i++) {
        pValue = toPyTuple(vec[i]);
        if (!pValue) {
            // If invalid conversion to vector, raise error
#ifdef DEBUG_MODE
            Logger::LogError("Cannot convert argument for tuple");
#endif
        }
        // Steals reference, so no need to DECREF
        PyTuple_SetItem(pTuple, i, pValue);
    }
    return pTuple;
}

PyObject * PythonAPI::toPyTuple(const std::vector<float>& vec) const
{
    PyObject* pTuple = toPyTuple(vec.size());
    PyObject* pValue;
    for (unsigned int i = 0; i < vec.size(); i++) {
        pValue = toPyFloat(vec[i]);
        if (!pValue) {
            // If invalid conversion to long, raise error
#ifdef DEBUG_MODE
            Logger::LogError("Cannot convert argument for tuple");
#endif
        }
        // Steals reference, so no need to DECREF
        PyTuple_SetItem(pTuple, i, pValue);
    }
    return pTuple;
}

bool PythonAPI::isTuple(PyObject * o) const
{
    if (o) { 
        return PyTuple_Check(o); 
    }
    else {
        return false;
    }
}

bool PythonAPI::isList(PyObject * o) const
{
    if (o) {
        return PyList_Check(o);
    }
    else {
        return false;
    }
}

bool PythonAPI::isListLike(PyObject * o) const
{
    return isList(o) || isTuple(o);
}

bool PythonAPI::isModule(PyObject * o) const
{
    if (o) {
        return PyModule_Check(o);
    }
    else {
        return false;
    }
}

py::object PythonAPI::toPyDict(const nlohmann::json& json) const
{
    // Follow pattern from PythonQt
//    const QVariantMap& kwargs = json.toObject().toVariantMap();
//    PyObject* dct = PyDict_New();
//
//    // Convert keyword arguments to Python objects
//    QMapIterator<QString, QVariant> it(kwargs);
//    while (it.hasNext()) {
//        it.next();
//        // Convert arg to a python object
//        PyObject* arg = PythonQtConv::QVariantToPyObject(it.value());
//        if (arg) {
//            // Add arg to the dictionary
//            PyDict_SetItemString(dct, QStringToPythonCharPointer(it.key()), arg);
//        }
//        else {
//#ifdef DEBUG_MODE
//            Logger::LogError("Error converting jsonValue to python dictionary");
//#endif
//            break;
//        }
//    }

    // NOTE: Do not mix exec calls if using py::module_ imports, be consistent
    GString str = GJson::ToString<GString>(json);
    const char* dctStr = str.c_str();
    py::object dct = m_json.attr("loads")(dctStr);
    return dct;
}

PyObject * PythonAPI::toPyFloat(double d) const
{
    return PyFloat_FromDouble(d);
}

PyObject* PythonAPI::toPyLon(double d) const
{
    return PyLong_FromDouble(d);
}


PyObject* PythonAPI::toPyLon(int i) const
{
    return PyLong_FromLong(i);
}

PyObject* PythonAPI::toPyBool(bool b) const
{
    if (b) {
        return Py_True;
    }
    else {
        return Py_False;
    }
}

PyObject * PythonAPI::toPyStr(const GString & s) const
{
    const char* chars = s.c_str();
    PyObject* pStr = PyUnicode_DecodeFSDefault(chars);

    // Error check
    printAndClearErrors();
#ifdef DEBUG_MODE
    const char* errors = Py_FileSystemDefaultEncodeErrors;
    Logger::LogError("String encode errors" + GString(errors));
#endif

    return pStr;
}

QString PythonAPI::printAndClearErrors() const
{
    if (PyErr_Occurred()) {
        PyErr_Print();
        //QString err = getStdErr();
        //Logger::LogError(err);
        clearStdErr();
        //return err;
    }
    return "";
}

bool PythonAPI::initializeMainModule()
{
    m_main = py::module_::import("__main__");
    if (!m_main) {
        printAndClearErrors();
        return false; 
    }
    else { 
        if (!isModule(m_main.ptr())) {
            Logger::Throw("Error, main is not a module");
        }
        return true; 
    }
}

//void PythonAPI::ensureMainModule()
//{
//    if (!m_main) {
//        bool madeMain = initializeMainModule();
//        if (!madeMain) {
//            printAndClearErrors();
//            Logger::Throw("Error, failed to initialize main module");
//        }
//
//    }    
//}
//void PythonAPI::initializeStdOut()
//{
//    // Create main module if it does not exist
//    ensureMainModule();
//
//    // Add sys to main
//    bool added = addModuleToMain("sys");
//    if (!added) {
//        Logger::Throw("Error, failed to import sys");
//    }
//
//    // Code to catch std::out
//    GString catcherCode(
//        "# catcher code\n"
//        //"import sys\n"
//        "class StdoutCatcher :\n"
//        "   def __init__(self) :\n"
//        "       self.data = ''\n"
//        "   def write(self, stuff) :\n"
//        "       self.data += stuff\n"
//        "   def clear(self) :\n"
//        "       self.data = ''\n"
//        "gb_catcher = StdoutCatcher()\n"
//        "sys.stdout = gb_catcher\n");
//
//    // Code to catch std::err
//    GString errorCatcherCode(
//        "# error catcher code\n"
//        "class StderrCatcher :\n"
//        "   def __init__(self) :\n"
//        "       self.data = ''\n"
//        "   def write(self, stuff) :\n"
//        "       self.data += stuff\n"
//        "   def clear(self) :\n"
//        "       self.data = ''\n"
//        "gb_err_catcher = StderrCatcher()\n"
//        "sys.stderr = gb_err_catcher\n");
//
//    // Run catcher code
//    bool ran = runCode(catcherCode);
//    if (!ran) {
//        Logger::Throw("Error, failed to initialize python std::out redirection");
//    }
//    m_outCatcher = getAttribute(m_mainModule, "gb_catcher");
//
//    // Run error catcher code
//    ran = runCode(errorCatcherCode);
//    if (!ran) {
//        Logger::Throw("Error, failed to initialize python std::err redirection");
//    }
//    m_errCatcher = getAttribute(m_mainModule, "gb_err_catcher");
//}

//GString PythonAPI::getStdOut() const
//{
//    // Get catcher containing std::output
//    PyObject* output = getAttribute(m_outCatcher, "data");
//    GString outputStr = cType<GString>(output).replace("\\n", "\n");
//    outputStr = outputStr.replace("'", "");
//    Py_DECREF(output);
//    return outputStr;
//}

//GString PythonAPI::getStdErr() const
//{
//    PyObject* output = getAttribute(m_errCatcher, "data");
//    GString outputStr = cType<GString>(output).replace("\\n", "\n");
//    outputStr = outputStr.replace("'", "");
//    Py_DECREF(output);
//    return outputStr;
//}

void PythonAPI::clearStdOut() const
{
    runCode("gb_catcher.clear()");
}

void PythonAPI::clearStdErr() const
{
    runCode("gb_err_catcher.clear()");
}

void PythonAPI::initialize(const GString& programName)
{
    // Interpreter is initialized as a member

    // Set script path
    const QString scriptDir = GetScriptDir();
    QDir::addSearchPath("py_scripts", scriptDir);

    // Initialize python program
    initializeProgram(programName);

    // Initialize main module 
    bool madeMain = initializeMainModule();
    if (!madeMain) {
        Logger::Throw("Error, failed to initialize main module");
    }

    // Import modules -------------------------------------------------
    // Import sys
    m_sys = py::module_::import("sys");

    // Import JSON
    m_json = py::module_::import("json");

    // Initialize std::out redirection from python
    // TODO: Redirect std::out to console widget, see:
    // https://github.com/pybind/pybind11/issues/1622
    //initializeStdOut();

    // Append the Qt resource scripts directory to the system path
    //py::print(m_sys.attr("path"));
    //m_sys.attr("path").attr("append")(":/scripts");
    //m_sys.attr("path").attr("append")(scriptDir.c_str());

    // Import reverie module
    m_reverie = py::module_::import("reverie");

    // Import Reverie classes ------------------------------------------
    // Import desired classes
    py::object mainScope = m_main.attr("__dict__");
    runCode(R"(from reverie import (ScriptBehavior, 
                                    ScriptListener, 
                                    SceneObject,
                                    Quaternion, EulerAngles,
                                    Vector2, Vector3, Vector4,
                                    Matrix2x2, Matrix3x3, Matrix4x4
                                   )
              )", mainScope);
}

void PythonAPI::initializeProgram(const GString& programName)
{
    /// \see https://docs.python.org/3.8/c-api/init.html#c.Py_SetProgramName
    m_programName = Py_DecodeLocale(programName.c_str(), NULL);
    if (m_programName == NULL) {
        // Terminate process if python terminal can't checkValidity
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
#ifdef DEBUG_MODE
        GString err("Fatal Error, failed to destruct Python");
        Logger::LogError(err);
        Logger::Throw(err);
#endif
    }

    //  It tells the interpreter the value of the argv[0] argument to the main() function of the program
    Py_SetProgramName(m_programName);  /* optional but recommended */

    // Initialize the python interpreter
    // Should be called before using any other Python/C API functions, besides:
    // Py_SetProgramName(), Py_SetPythonHome(), PyEval_InitThreads(), PyEval_ReleaseLock(), and PyEval_AcquireLock()
    // Initializes table of loaded modules (sys.modules), and creates fundamental 
    // modules __builtin__, __main__, sys. 
    // Also initializes the module search path (sys.path), does not set sys.argv
    // No return error, fatal error on failure

    // Incompatible with pybind, doesn't manage lifetime of objects properly
    /// \see https://pybind11.readthedocs.io/en/stable/advanced/embedding.html
    //if (!Py_IsInitialized()) {
    //    Py_Initialize();
    //}
}

std::unique_ptr<PythonAPI> PythonAPI::s_instance = nullptr;



// End namespaces
}