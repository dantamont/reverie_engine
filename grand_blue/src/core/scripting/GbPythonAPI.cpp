#include "GbPythonAPI.h"
#include <QDir>
#include "../../third_party/pythonqt/PythonQtConversion.h"

#include "../readers/GbJsonReader.h"
#include "../scripting/GbPyWrappers.h"
#include "../containers/GbContainerExtensions.h"

namespace Gb{

//////////////////////////////////////////////////////////////////////////////////////////////////////////
PythonAPI * PythonAPI::get()
{
    if (!INSTANCE) {
        INSTANCE = new PythonAPI(const_cast<char*>("PythonQt"));
    }

    return INSTANCE;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
QString PythonAPI::getScriptDir()
{
    QString dir = QDir::currentPath();
    //dir = QGuiApplication::applicationDirPath();
    QString scriptPath = QDir::cleanPath(dir + QDir::separator() + "resources/scripts");
    return scriptPath;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::PythonAPI::PythonAPI(const QString& programName):
    m_mainModule(nullptr),
    m_outCatcher(nullptr),
    m_errCatcher(nullptr)
{
    // Set script path
    QString scriptDir = getScriptDir();
    QDir::addSearchPath("py_scripts", scriptDir);

    // Initialize python program
    initializeProgram(programName);

    // Initialize main module 
    bool madeMain = initializeMainModule();
    if (!madeMain) throw("Error, failed to initialize main module");

    // Initialize std::out redirection from python
    initializeStdOut();

    // Append the Qt resource scripts directory to the system path
    addSysPath(":/scripts");
    addSysPath(scriptDir);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::PythonAPI::~PythonAPI()
{
    // Decrement main module reference
    Py_XDECREF(m_mainModule);

    // Undo all initializations made by Py_Initialize() and subsequent use of Python/C API 
    // functions, and destroy all sub-interpreters (see Py_NewInterpreter() below) 
    // that were created and not yet destroyed since the last call to Py_Initialize(). 
    // Ideally, this frees all memory allocated by the Python interpreter.
    // Returns 0 if bueno
    if (Py_FinalizeEx() < 0) {
#ifdef DEBUG_MODE
        logError("Error, failed to destruct Python");
#endif
    }

    // Free the memory block pointed to by program
    PyMem_RawFree(m_programName);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonAPI::clear()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject* PythonAPI::importModule(const QString & packageName) const
{
    // Get module name as python string
    PyObject* pPackageName = toPyStr(packageName);

    // Import module
    // This is a higher-level interface that calls the current “import hook function” 
    // (with an explicit level of 0, meaning absolute import). It invokes the
    // __import__() function from the __builtins__ of the current globals. 
    // This means that the import is done using whatever import hooks are 
    // installed in the current environment.
    // This function always uses absolute imports.

    // note:
    // Beware, in particular, of using function PyImport_ImportModule, which may often 
    // look more convenient because it accepts a char* argument. PyImport_ImportModule 
    // operates on a lower level, bypassing any import hooks that may be in force, 
    // so extensions that use it will be far harder to incorporate in packages 
    // such as those built by tools py2exe and Installer, covered in Chapter 26. 
    // Therefore, always do your importing by calling PyImport_Import, 
    // unless you have very specific needs and know exactly what you’re doing.

    //The import statement combines two operations;
    // it searches for the named module, then it binds the results of that search to 
    // a name in the local scope. The search operation of the import statement is
    // defined as a call to the __import__() function, with the appropriate arguments.
    // The return value of __import__() is used to perform the name binding operation 
    // of the import statement. 
    // See the import statement for the exact details of that name binding operation.

    // A direct call to __import__() performs only the module search and, if found, 
    // the module creation operation.
    PyObject* pPackage = PyImport_Import(pPackageName);

    // Decrement reference count to module name for garbage collection
    // See: https://stackoverflow.com/questions/6977161/where-should-i-put-py-incref-and-py-decref-on-this-block-in-python-c-extension
    Py_DECREF(pPackageName);

    // Check that module was imported properly
#ifdef DEBUG_MODE
    if (!pPackage) {
        // Print a standard traceback to sys.stderr and clear the error indicator.
        // Unless the error is a SystemExit, in that case no traceback is printed and 
        // the Python process will exit with the error code specified by the SystemExit instance.
        // Call this function only when the error indicator is set. Otherwise it will cause a fatal error!
        PyErr_Print();
        logError("Error, failed to load package " + packageName);
    }
#endif
    return pPackage;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PythonAPI::runCode(const QString & code) const
{
    std::string str = code.toStdString();
    int ran = PyRun_SimpleString(str.c_str());
    if (ran < 0) {
        printAndClearErrors();
        return false;
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PythonAPI::runFile(const QString & filepath) const
{
    // See: https://stackoverflow.com/questions/3654652/why-does-the-python-c-api-crash-on-pyrun-simplefile
    std::string fileStr = filepath.toStdString();
    PyObject *obj = Py_BuildValue("s", fileStr.c_str());
    FILE* file = _Py_fopen_obj(obj, "r+");
    int ran = PyRun_SimpleFile(file, fileStr.c_str());
    if (ran < 0) {
        printAndClearErrors();
        return false;
    }
    Py_DECREF(obj);
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
QVariant PythonAPI::toQVariant(PyObject * val, int type)
{
    QVariant variant = PythonQtConv::PyObjToQVariant(val, type);
#ifdef DEBUG_MODE
    if (!variant.isValid()) {
        int type = variant.userType();
        QString typeStr = QVariant::typeToName(type);
        logError("Error, QVariant of type " + typeStr + " is invalid.");
    }
#endif
    return variant;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::getClass(const QString & className) const
{
    PyObject* class_ = getVariable(className);
    if (!class_) {
        throw("Error, no class found");
    }
    if (!isClass(class_)) {
        throw("Error, object is not a class");
    }
    return class_;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PythonAPI::isClass(PyObject * cls) const
{
    if (cls) {
        return PyType_Check(cls);
    }
    return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
QString PythonAPI::getClassName(PyObject * o) const
{
    return QString(o->ob_type->tp_name);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::instantiate(const QString & className, PyObject * argList) const
{
    // Call the class object.
    PyObject* cls = getClass(className);
    PyObject* obj = PyObject_CallObject(cls, argList);

    if (!obj) {
        QString argDictInfo = getDictStr(argList);
        std::vector<QString> argInfo = getInfo(argList);
        printAndClearErrors();
#ifdef DEBUG_MODE
        QString errStr = "Bad call to " + className + " constructor, with args" + argDictInfo;
        throw(errStr);
#endif
    }

    // Release the argument list.
    Py_XDECREF(argList);

    // Release the class string
    Py_DECREF(cls);

    return obj;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PythonAPI::isSubclass(const QString & derivedName, const QString & clsName) const
{
    PyObject* derived = getClass(derivedName);
    PyObject* cls = getClass(clsName);
    return isSubclass(derived, cls);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PythonAPI::isSubclass(PyObject * derived, PyObject * cls) const
{
    // Determine whether or not derived is a subclass of cls
    bool isSub = false;
    if (isClass(derived) && isClass(cls)) {
        isSub = PyObject_IsSubclass(derived, cls);
    }
    else {
        QString errStr = "isSubclass:: Error, given objects are not classes";
        logError(errStr);
#ifdef DEBUG_MODE
        throw(errStr);
#endif
    }

    // Decrement class strings
    Py_DECREF(derived);
    Py_DECREF(cls);

    return isSub;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<QString> PythonAPI::getInfo(PyObject * o) const
{
    PyObject* info = PyObject_Dir(o);
    printAndClearErrors();
    return toVecStr(info);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
QString PythonAPI::getDictStr(PyObject* o) const
{
    return toStr(getDict(o));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::getDict(PyObject * o) const
{
    return PyObject_GenericGetDict(o, NULL);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::getFunction(PyObject * o, const QString & attrName) const
{
    PyObject* callable = getAttribute(o, attrName);
    if (isCallableAttribute(callable)) {
        return callable;
    }
    else {
        // Print error if occurred
        printAndClearErrors();
#ifdef DEBUG_MODE
        logError("Attribute is not callable");
#endif
        Py_XDECREF(callable);
    }
    return nullptr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::getVariable(const QString & variableName) const
{
    return getAttribute(m_mainModule, variableName);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonAPI::addSysPath(const QString & path) const
{
    QString addPathLine = QStringLiteral("sys.path.append('") + path + "')\n";
    runCode(addPathLine);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::getAttribute(PyObject * o, const QString & attrName) const
{
    // Convert QString to c_str
    std::string str = attrName.toStdString();
    const char* c = str.c_str();

    // Same as o.attr_name, so need to DECREF when done with attr
    if (!o) {
        QString err = getStdErr();
        QString errStr = "Error, python object is null: " + err;
        logError(errStr);
#ifdef DEBUG_MODE
        throw(errStr);
#endif
    }
    PyObject* attr = PyObject_GetAttrString(o, c);
    if (attr == NULL) {
        printAndClearErrors();
        //throw("error, no attribute obtained");
        return nullptr;
    }
    else {
        return attr;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::call(const QString & objectName, const QString & methodName, const std::vector<GVariant>& args) const
{
    QString typeStr;
    PyObject* result;
    PyObject* pyArgs = PyList_New(args.size());
    int setItem;
    int count = 0;
    for (const GVariant& arg: args) {
        // Item to add to argument list
        PyObject* item;

        // Get type for format str
        if (arg.is<int>()) {
            typeStr.append("i");
            item = toPyLon(arg.get<int>());
        }
        else if (arg.is<QString>()) {
            typeStr.append("s");
            item = toPyStr(arg.get<QString>());
        }
        else {
            QString err = "call::Error, python argument type not recognized";
            logError(err);
#ifdef DEBUG_MODE
            throw(err);
#endif
        }

        // If typestring is a size of 1, force tuple
        if (typeStr.size() == 1) {
            typeStr = "(" + typeStr + ")";
        }

        // Add to python argument list
        setItem = PyList_SetItem(pyArgs, count, item);
        if (setItem < 0) {
            QString err = "Error, failed to set tuple item";
            logError(err);
#ifdef DEBUG_MODE
            throw(err);
#endif
        }
        count++;
    }

    // Call method
    result = call(objectName, methodName, pyArgs);

    std::vector<QString> argList = toVecStr(pyArgs);

    printAndClearErrors();
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::call(const QString & objectName, const QString & methodName, PyObject * args) const
{
    // Get object from object name
    PyObject* o = getVariable(objectName);

    // Get function name and call
    PyObject* pFunc = getFunction(o, methodName);
    PyObject* result = call(pFunc, args);

    Py_DECREF(o);
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::call(PyObject * o, const QString & methodName, PyObject * args) const
{
    // Get function name and call
    PyObject* pFunc = getFunction(o, methodName);
    PyObject* result = call(pFunc, args);
    //std::string str = methodName.toStdString();
    //PyObject* result = PyObject_CallMethod(o, str.c_str());
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::call(PyObject * o) const
{
    return call(o, NULL);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::call(PyObject * o, PyObject * args) const
{
    PyObject* output = nullptr;
    if (isCallableAttribute(o) && (isTuple(args) || args == NULL)) {
        output = PyObject_CallObject(o, args);
        if (output == NULL) {
#ifdef DEBUG_MODE
            logError("Call failed");
#endif  
        }
    }
    else {
        // Print error if occurred
        printAndClearErrors();
#ifdef DEBUG_MODE
        logError("Cannot find callable, or args are not a tuple");
#endif
    }

    // Decrement count for function, ignoring if it is null
    Py_XDECREF(o);

    return output;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PythonAPI::isCallableAttribute(PyObject * o) const
{
    return o && PyCallable_Check(o);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::toPyTuple(size_t len) const
{
    return PyTuple_New(len);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::toPyTuple(const std::vector<int>& vec) const
{
    PyObject* pTuple = toPyTuple(vec.size());
    PyObject* pValue;
    for (unsigned int i = 0; i < vec.size(); i++) {
        pValue = toPyLon(vec[i]);
        if (!pValue) {
            // If invalid conversion to long, raise error
#ifdef DEBUG_MODE
            logError("Cannot convert argument for tuple");
#endif
        }
        // Steals reference, so no need to DECREF
        PyTuple_SetItem(pTuple, i, pValue);
    }
    return pTuple;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::toPyTuple(const std::vector<double>& vec) const
{
    PyObject* pTuple = toPyTuple(vec.size());
    PyObject* pValue;
    for (unsigned int i = 0; i < vec.size(); i++) {
        pValue = toPyFloat(vec[i]);
        if (!pValue) {
            // If invalid conversion to long, raise error
#ifdef DEBUG_MODE
            logError("Cannot convert argument for tuple");
#endif
        }
        // Steals reference, so no need to DECREF
        PyTuple_SetItem(pTuple, i, pValue);
    }
    return pTuple;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::toPyTuple(const std::vector<std::vector<double>>& vec) const
{
    PyObject* pTuple = toPyTuple(vec.size());
    PyObject* pValue;
    for (unsigned int i = 0; i < vec.size(); i++) {
        pValue = toPyTuple(vec[i]);
        if (!pValue) {
            // If invalid conversion to vector, raise error
#ifdef DEBUG_MODE
            logError("Cannot convert argument for tuple");
#endif
        }
        // Steals reference, so no need to DECREF
        PyTuple_SetItem(pTuple, i, pValue);
    }
    return pTuple;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::toPyTuple(const std::vector<std::vector<float>>& vec) const
{
    PyObject* pTuple = toPyTuple(vec.size());
    PyObject* pValue;
    for (unsigned int i = 0; i < vec.size(); i++) {
        pValue = toPyTuple(vec[i]);
        if (!pValue) {
            // If invalid conversion to vector, raise error
#ifdef DEBUG_MODE
            logError("Cannot convert argument for tuple");
#endif
        }
        // Steals reference, so no need to DECREF
        PyTuple_SetItem(pTuple, i, pValue);
    }
    return pTuple;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::toPyTuple(const std::vector<float>& vec) const
{
    PyObject* pTuple = toPyTuple(vec.size());
    PyObject* pValue;
    for (unsigned int i = 0; i < vec.size(); i++) {
        pValue = toPyFloat(vec[i]);
        if (!pValue) {
            // If invalid conversion to long, raise error
#ifdef DEBUG_MODE
            logError("Cannot convert argument for tuple");
#endif
        }
        // Steals reference, so no need to DECREF
        PyTuple_SetItem(pTuple, i, pValue);
    }
    return pTuple;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::vector<double>> PythonAPI::toVecOfVecDouble(PyObject * incoming) const
{
    std::vector<std::vector<double>> data;
    if (isTuple(incoming)) {
        for (Py_ssize_t i = 0; i < PyTuple_Size(incoming); i++) {
            // PyTyple_GetItem does not call INCREF (unlike many functions that return an item)
            // So NO reference decrementing
            PyObject *value = PyTuple_GetItem(incoming, i);
            Vec::EmplaceBack(data, toVecDouble(value));
        }
    }
    else {
        if (isList(incoming)) {
            for (Py_ssize_t i = 0; i < PyList_Size(incoming); i++) {
                PyObject *value = PyList_GetItem(incoming, i);
                Vec::EmplaceBack(data, toVecDouble(value));
            }
        }
        else {
            throw("Passed PyObject pointer was not a list or tuple!");
        }
    }
    return data;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<double> PythonAPI::toVecDouble(PyObject * incoming) const
{
    std::vector<double> data;
    if (isTuple(incoming)) {
        for (Py_ssize_t i = 0; i < PyTuple_Size(incoming); i++) {
            // PyTyple_GetItem does not call INCREF (unlike many functions that return an item)
            // So NO reference decrementing
            PyObject *value = PyTuple_GetItem(incoming, i);
            Vec::EmplaceBack(data, toDouble(value));
        }
    }
    else {
        if (isList(incoming)) {
            for (Py_ssize_t i = 0; i < PyList_Size(incoming); i++) {
                PyObject *value = PyList_GetItem(incoming, i);
                Vec::EmplaceBack(data, toDouble(value));
            }
        }
        else {
            throw("Passed PyObject pointer was not a list or tuple!");
        }
    }
    return data;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<float> PythonAPI::toVecFloat(PyObject * incoming) const
{
    std::vector<float> data;
    if (isTuple(incoming)) {
        for (Py_ssize_t i = 0; i < PyTuple_Size(incoming); i++) {
            // PyTyple_GetItem does not call INCREF (unlike many functions that return an item)
            // So NO reference decrementing
            PyObject *value = PyTuple_GetItem(incoming, i);
            Vec::EmplaceBack(data, toDouble(value));
        }
    }
    else {
        if (isList(incoming)) {
            for (Py_ssize_t i = 0; i < PyList_Size(incoming); i++) {
                PyObject *value = PyList_GetItem(incoming, i);
                Vec::EmplaceBack(data, toDouble(value));
            }
        }
        else {
            throw("Passed PyObject pointer was not a list or tuple!");
        }
    }
    return data;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<int> PythonAPI::toVecInt(PyObject * incoming) const
{
    std::vector<int> data;
    if (isTuple(incoming)) {
        for (Py_ssize_t i = 0; i < PyTuple_Size(incoming); i++) {
            // PyTyple_GetItem does not call INCREF (unlike many functions that return an item)
            // So NO reference decrementing
            PyObject *value = PyTuple_GetItem(incoming, i);
            Vec::EmplaceBack(data, int(PyInt_AsLong(value)));
        }
    }
    else {
        if (isList(incoming)) {
            for (Py_ssize_t i = 0; i < PyList_Size(incoming); i++) {
                PyObject *value = PyList_GetItem(incoming, i);
                Vec::EmplaceBack(data, int(PyInt_AsLong(value)));
            }
        }
        else {
            throw("Passed PyObject pointer was not a list or tuple!");
        }
    }
    return data;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<QString> PythonAPI::toVecStr(PyObject * incoming) const
{
    std::vector<QString> data;
    if (isTuple(incoming)) {
        for (Py_ssize_t i = 0; i < PyTuple_Size(incoming); i++) {
            PyObject *value = PyTuple_GetItem(incoming, i);
            Vec::EmplaceBack(data, toStr(value));
        }
    }
    else {
        if (isList(incoming)) {
            for (Py_ssize_t i = 0; i < PyList_Size(incoming); i++) {
                PyObject *value = PyList_GetItem(incoming, i);
                Vec::EmplaceBack(data, toStr(value));
            }
        }
        else {
            throw("Passed PyObject pointer was not a list or tuple!");
        }
    }
    return data;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PythonAPI::isTuple(PyObject * o) const
{
    if (o) { 
        return PyTuple_Check(o); 
    }
    else {
        return false;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PythonAPI::isList(PyObject * o) const
{
    if (o) {
        return PyList_Check(o);
    }
    else {
        return false;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PythonAPI::isListLike(PyObject * o) const
{
    return isList(o) || isTuple(o);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PythonAPI::isModule(PyObject * o) const
{
    if (o) {
        return PyModule_Check(o);
    }
    else {
        return false;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::toPyDict(const QJsonValue & json) const
{
    // Follow pattern from PythonQt
    const QVariantMap& kwargs = json.toObject().toVariantMap();
    PyObject* dct = PyDict_New();

    // Convert keyword arguments to Python objects
    QMapIterator<QString, QVariant> it(kwargs);
    while (it.hasNext()) {
        it.next();
        // Convert arg to a python object
        PyObject* arg = PythonQtConv::QVariantToPyObject(it.value());
        if (arg) {
            // Add arg to the dictionary
            PyDict_SetItemString(dct, QStringToPythonCharPointer(it.key()), arg);
        }
        else {
#ifdef DEBUG_MODE
            logError("Error converting jsonValue to python dictionary");
#endif
            break;
        }
    }

    return dct;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::toPyFloat(double d) const
{
    return PyFloat_FromDouble(d);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject* PythonAPI::toPyLon(double d) const
{
    return PyLong_FromDouble(d);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject* PythonAPI::toPyLon(int i) const
{
    return PyLong_FromLong(i);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject* PythonAPI::toPyBool(bool b) const
{
    if (b) {
        return Py_True;
    }
    else {
        return Py_False;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * PythonAPI::toPyStr(const QString & s) const
{
    std::string stdStr = s.toStdString();
    const char* chars = stdStr.c_str();
    PyObject* pStr = PyUnicode_DecodeFSDefault(chars);

    // Error check
    printAndClearErrors();
#ifdef DEBUG_MODE
    const char* errors = Py_FileSystemDefaultEncodeErrors;
    logError("String encode errors" + QString(errors));
#endif

    return pStr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
QString PythonAPI::toStr(PyObject * o) const
{
    PyObject* rep = PyObject_Repr(o);
    const char * s = PyUnicode_AsUTF8(rep);
    if (s == NULL) {
        printAndClearErrors();
        throw("Error, string conversion failed");
    }

    QString out(s);
    Py_XDECREF(rep);
    return out;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
double PythonAPI::toDouble(PyObject * o) const
{
    return PyFloat_AsDouble(o);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
QString PythonAPI::printAndClearErrors() const
{
    if (PyErr_Occurred()) {
        PyErr_Print();
        QString err = getStdErr();
        logError(err);
        clearStdErr();
        return err;
    }
    return "";
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PythonAPI::addModuleToMain(const QString& packageName)
{
    // Create main module if it does not exist
    ensureMainModule();

    // Import package
    PyObject* module = importModule(packageName);

    // Set package in main
    std::string packageStdStr = packageName.toStdString();
    const char* packChars = packageStdStr.c_str();
    PyObject_SetAttrString(m_mainModule, packChars, module);

    //PyObject *packageDict = PyModule_GetDict(module);
    //PyObject *mainDict = PyModule_GetDict(m_mainModule);
    //PyDict_SetItemString(mainDict, packChars, module);
    
    Py_XDECREF(module);
    //Py_DECREF(mainDict);
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonAPI::ensureMainModule()
{
    if (!m_mainModule) {
        bool madeMain = initializeMainModule();
        if (!madeMain) {
            printAndClearErrors();
            throw("Error, failed to initialize main module");
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PythonAPI::initializeMainModule()
{
    // Return the module object corresponding to a module name.
    // The name argument may be of the form package.module.
    // First check the modules dictionary if there’s one there, and if not, 
    // create a new one and insert it in the modules dictionary.
    // Return NULL with an exception set on failure.
    m_mainModule = PyImport_AddModule("__main__");

    if (!m_mainModule) { 
        printAndClearErrors();
        return false; 
    }
    else { 
        if (!isModule(m_mainModule)) {
            throw("Error, main is not a module");
        }
        return true; 
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonAPI::initializeStdOut()
{
    // Create main module if it does not exist
    ensureMainModule();

    // Add sys to main
    bool added = addModuleToMain("sys");
    if (!added) {
        throw("Error, failed to import sys");
    }

    // Code to catch std::out
    QString catcherCode(
        "# catcher code\n"
        //"import sys\n"
        "class StdoutCatcher :\n"
        "   def __init__(self) :\n"
        "       self.data = ''\n"
        "   def write(self, stuff) :\n"
        "       self.data += stuff\n"
        "   def clear(self) :\n"
        "       self.data = ''\n"
        "gb_catcher = StdoutCatcher()\n"
        "sys.stdout = gb_catcher\n");

    // Code to catch std::err
    QString errorCatcherCode(
        "# error catcher code\n"
        "class StderrCatcher :\n"
        "   def __init__(self) :\n"
        "       self.data = ''\n"
        "   def write(self, stuff) :\n"
        "       self.data += stuff\n"
        "   def clear(self) :\n"
        "       self.data = ''\n"
        "gb_err_catcher = StderrCatcher()\n"
        "sys.stderr = gb_err_catcher\n");

    // Run catcher code
    bool ran = runCode(catcherCode);
    if (!ran) {
        throw("Error, failed to initialize python std::out redirection");
    }
    m_outCatcher = getAttribute(m_mainModule, "gb_catcher");

    // Run error catcher code
    ran = runCode(errorCatcherCode);
    if (!ran) {
        throw("Error, failed to initialize python std::err redirection");
    }
    m_errCatcher = getAttribute(m_mainModule, "gb_err_catcher");
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonAPI::logStdOut()
{
    // Log std::out
    QString stdOut = getStdOut();
    if (stdOut.size() > 2) {
        clearStdOut();
        m_stdOut += stdOut;
    }

    // Display out if enough time has passed
    if (m_timer.elapsed() > 1000) {
        m_timer.restart();
        
        if (!m_stdOut.isEmpty()) logInfo(m_stdOut);
        m_stdOut = "";
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
QString PythonAPI::getStdOut() const
{
    // Get catcher containing std::output
    PyObject* output = getAttribute(m_outCatcher, "data");
    QString outputStr = toStr(output).replace("\\n", "\n");
    outputStr = outputStr.replace("'", "");
    Py_DECREF(output);
    return outputStr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
QString PythonAPI::getStdErr() const
{
    PyObject* output = getAttribute(m_errCatcher, "data");
    QString outputStr = toStr(output).replace("\\n", "\n");
    outputStr = outputStr.replace("'", "");
    Py_DECREF(output);
    return outputStr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonAPI::clearStdOut() const
{
    runCode("gb_catcher.clear()");
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonAPI::clearStdErr() const
{
    runCode("gb_err_catcher.clear()");
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PythonAPI::initializeProgram(const QString& programName)
{
    std::string stdStr = programName.toStdString();
    m_programName = Py_DecodeLocale(stdStr.c_str(), NULL);
    if (m_programName == NULL) {
        // Terminate process if python terminal can't checkValidity
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
#ifdef DEBUG_MODE
        QString err("Fatal Error, failed to destruct Python");
        logError(err);
        throw(err);
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
    if (!Py_IsInitialized()) {
        Py_Initialize();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
PythonAPI* PythonAPI::INSTANCE = nullptr;


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}