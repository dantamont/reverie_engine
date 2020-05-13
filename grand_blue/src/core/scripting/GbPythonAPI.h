/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_PYTHON_API_H
#define GB_PYTHON_API_H

// External
#include "GbPythonWrapper.h" // include before standard headers

// Qt
#include <QString>
#include <QElapsedTimer>

// Internal
#include "../containers/GbGVariant.h"
#include "../GbObject.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class PythonAPI
/// @brief Represents custom-built API for embedding Python into the application
/// @details See:
/// https://docs.python.org/3.8/extending/embedding.html
/// https://stackoverflow.com/questions/1056051/how-do-you-call-python-code-from-c-code
class PythonAPI : public Object {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    static PythonAPI* get();

    /// @brief Returns directory where scripts reside
    static QString getScriptDir();


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Return main module
    PyObject* mainModule() const { return m_mainModule; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Clear any objects cached by the API
    void clear();

    /// @brief Import a Python script, using an absolute import
    /// @details Invokes the __import__() function from the __builtins__ of the current globals
    /// @note Just reterns a reference to the module, does not make the module available
    /// to other parts of the program
    PyObject* importModule(const QString& packageName) const;

    /// @brief Run a simple python string
    bool runCode(const QString& code) const;

    /// @brief Run python code from a filepath
    bool runFile(const QString& filepath) const;

    /// @brief Convert the given pyobject to a QVariant type, if possible
    QVariant toQVariant(PyObject* val, int type = -1);

    /// @brief Obtain a pointer to a class in python
    /// @note See: https://stackoverflow.com/questions/21929143/some-confustion-about-pyclass-check-and-pyclass-issubclass-functions
    PyObject* getClass(const QString& className) const;
    bool isClass(PyObject* class_) const;
    QString getClassName(PyObject* o) const;

    /// @brief Instantiate a class in python
    /// @note See: https://stackoverflow.com/questions/40351244/create-an-instance-of-python-new-style-class-to-embed-in-c
    /// https://stackoverflow.com/questions/4163018/create-an-object-using-pythons-c-api
    // https://docs.python.org/3/c-api/arg.html
    PyObject* instantiate(const QString& className, PyObject* argList = NULL) const;

    /// @brief Whether or not derived is a subclass of cls
    /// @note to get class of an instance, call instance->ob_type->tp_base
    bool isSubclass(const QString& derivedName, const QString& clsName) const;
    bool isSubclass(PyObject* derived, PyObject* cls) const;

    /// @brief use dir to get info about python object
    std::vector<QString> getInfo(PyObject* o) const;

    /// @brief Get info about the given PyObject via its __dict__
    QString getDictStr(PyObject* o) const;

    /// @brief Get the __dict__ attribute of the given PyObject
    PyObject* getDict(PyObject* o) const;

    /// @brief Get an attribute from a PyObject and ensure it is callable
    PyObject* getFunction(PyObject* o, const QString& attrName) const;

    /// @brief Get a variable from the main module
    PyObject* getVariable(const QString& variableName) const;

    /// @brief Get an attribute from a PyObject
    PyObject* getAttribute(PyObject* o, const QString& attrName) const;

    /// @brief Call a method of a PyObject
    PyObject* call(const QString& objectName, const QString& methodName, const std::vector<GVariant>& args) const;
    PyObject* call(const QString& objectName, const QString& methodName, PyObject* args) const;
    PyObject* call(PyObject* o, const QString& methodName, PyObject* args=NULL) const;

    /// @brief Call a callable object
    PyObject* call(PyObject* o) const;
    PyObject* call(PyObject* o, PyObject* args) const;

    /// @brief Check if an object is a callable attribute
    bool isCallableAttribute(PyObject* o) const;

    /// @brief Create a python tuple of length len
    PyObject* toPyTuple(size_t len) const;
    PyObject* toPyTuple(const std::vector<int>& vec) const;
    PyObject* toPyTuple(const std::vector<double>& vec) const;
    PyObject* toPyTuple(const std::vector<float>& vec) const;
    PyObject* toPyTuple(const std::vector<std::vector<float>>& vec) const;
    PyObject* toPyTuple(const std::vector<std::vector<double>>& vec) const;

    std::vector<std::vector<double>> toVecOfVecDouble(PyObject* incoming) const;
    std::vector<double> toVecDouble(PyObject* incoming) const;
    std::vector<float> toVecFloat(PyObject* incoming) const;
    std::vector<int> toVecInt(PyObject* incoming) const;
    std::vector<QString> toVecStr(PyObject* incoming) const;

    /// @brief Check if an object is a tuple
    bool isTuple(PyObject* o) const;
    bool isList(PyObject* o) const;
    bool isListLike(PyObject* o) const;
    bool isModule(PyObject* o) const;

    /// Create a python dict
    PyObject* toPyDict(const QJsonValue& json) const;

    /// Create a python double
    PyObject* toPyFloat(double d) const;

    /// @brief Create a python long
    PyObject* toPyLon(double d) const;
    PyObject* toPyLon(int i) const;

    /// @brief Create a python bool object
    PyObject* toPyBool(bool b) const;

    /// @brief Create a python string
    PyObject* toPyStr(const QString& s) const;

    /// @brief Create a C string from python string
    QString toStr(PyObject* o) const;

    /// @brief Create a double from a python object
    double toDouble(PyObject* o) const;

    /// @brief Print errors, returns true if errors were found
    QString printAndClearErrors() const;

    /// @brief Add a directory to system path for python imports
    void addSysPath(const QString& path) const;

    /// @brief Add a module to the main module
    /// @details Same as "import packageName"
    /// See: https://stackoverflow.com/questions/878439/pyimport-import-vs-import/878790
    bool addModuleToMain(const QString& packageName);

    /// @brief Instantiate main module if it doesn't exist
    void ensureMainModule();

    /// @brief Instantiate main module
    /// @details For importing packages, see:
    /// See: 
    /// https://stackoverflow.com/questions/28295674/define-python-class-from-c
    bool initializeMainModule();

    /// @brief Set up std::out to C++
    /// @details: See: https://stackoverflow.com/questions/4307187/how-to-catch-python-stdout-in-c-code
    /// http://mateusz.loskot.net/post/2011/12/01/python-sys-stdout-redirection-in-cpp/
    void initializeStdOut();

    /// @brief Log and clear all std::out/std::err contents
    void logStdOut();

    /// @brief Get std::out contents
    QString getStdOut() const;

    /// @brief Get std::err contents
    QString getStdErr() const;

    /// @brief Clear std::out contents
    void clearStdOut() const;

    /// @brief Clear std::err contents
    void clearStdErr() const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Loadable Overrides
    /// @{
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{
        /// @property className
    virtual const char* className() const { return "PythonAPI"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::PythonAPI"; }
    /// @}


protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    PythonAPI(const QString& programName = "python_gb");
    ~PythonAPI();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Initialize python interpreter
    void initializeProgram(const QString& programName);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief QString representing the python program
    wchar_t * m_programName;

    /// @brief Main module
    PyObject* m_mainModule;

    /// @brief Pointer to the only Singleton PythonAPI instance
    static PythonAPI* INSTANCE;

    /// @brief Pointers to the std::out and std::err catchers
    PyObject* m_outCatcher;
    PyObject* m_errCatcher;

    /// @brief std::out and std::err messages to be logged
    QString m_stdOut;

    /// @brief Timer for logging
    QElapsedTimer m_timer;

    /// @}
};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif