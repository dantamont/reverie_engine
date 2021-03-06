/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_PYTHON_API_H
#define GB_PYTHON_API_H

// std
#include <memory>

// External
#include "GPythonWrapper.h" // everything needed for embedding (needs to be included before Qt stuff pollutes slots)

// Qt
#include <QString>
#include <QElapsedTimer>

// Internal
#include "../containers/GString.h"
#include "../GObject.h"
#include "../geometry/GMatrix.h"

namespace py = pybind11;

namespace rev {
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
/// https://pybind11.readthedocs.io/en/stable/advanced/pycpp/object.html#casting-back-and-forth
class PythonAPI : public Object {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    static PythonAPI* get();

    /// @brief Returns directory where scripts reside
    static QString GetScriptDir();

    /// @brief Clear the python interpreter and restart
    static void Clear();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Return main module
    const py::module_& mainModule() const { return m_main; }

    const py::module_& reverieModule() const { return m_reverie; }
    const py::module_& componentsModule() const { return m_reverie.attr("components"); }

    /// @}


    //--------------------------------------------------------------------------------------------
    /// @name Destructor
    /// @{

    ~PythonAPI();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Import a Python script, using an absolute import
    /// @note See: https://stackoverflow.com/questions/878439/pyimport-import-vs-import/878790
    py::module_ importModule(const GString& packageName) const;

    /// @brief Run a simple python string
    void runCode(const GString& code) const;
    void runCode(const GString& code, py::object& scope) const;

    /// @brief Run python code from a filepath
    void runFile(const GString& filepath) const;

    /// @brief Convert the given pyobject to a QVariant type, if possible
    //QVariant toQVariant(PyObject* val, int type = -1);

    /// @brief Obtain a pointer to a class in python
    /// @note See: https://stackoverflow.com/questions/21929143/some-confustion-about-pyclass-check-and-pyclass-issubclass-functions
    py::object getClass(const GString& className) const;
    bool isClass(const py::object& class_) const;
    GString getClassName(const py::object& o) const;

    /// @brief Instantiate a class in python
    /// @note See: https://stackoverflow.com/questions/40351244/create-an-instance-of-python-new-style-class-to-embed-in-c
    /// https://stackoverflow.com/questions/4163018/create-an-object-using-pythons-c-api
    // https://docs.python.org/3/c-api/arg.html
    py::object instantiate(const GString& className, const py::object& args) const;

    /// @brief Whether or not derived is a subclass of cls
    /// @note to get class of an instance, call instance->ob_type->tp_base
    bool isSubclass(const GString& derivedName, const GString& clsName) const;
    bool isSubclass(const py::object& derived, const py::object& cls) const;

    /// @brief use dir to get info about python object
    std::vector<GString> getInfo(PyObject* o) const;

    /// @brief Get info about the given PyObject via its __dict__
    GString getDictStr(PyObject* o) const;

    /// @brief Get the __dict__ attribute of the given PyObject
    PyObject* getDict(PyObject* o) const;

    /// @brief Get an attribute from a PyObject and ensure it is callable
    PyObject* getFunction(PyObject* o, const GString& attrName) const;

    /// @brief Get a variable from the main module
    py::object getVariable(const GString& variableName) const;

    /// @brief Get an attribute from a PyObject
    PyObject* getAttribute(PyObject* o, const GString& attrName) const;

    /// @brief Call a method of a PyObject
    PyObject* call(const GString& objectName, const GString& methodName, PyObject* args) const;
    PyObject* call(PyObject* o, const GString& methodName, PyObject* args=NULL) const;

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

    template<typename T, size_t R, size_t C>
    void toMatrix(PyObject* incoming, Matrix<T, R, C>& outMatrix) const {
        if (isTuple(incoming)) {
            for (size_t i = 0; i < C; i++) {
                // PyTyple_GetItem does not call INCREF (unlike many functions that return an item)
                // So NO reference decrementing
                PyObject* column = PyTuple_GetItem(incoming, i);
                toVec(column, outMatrix.column(i));
            }
        }
        else {
            if (isList(incoming)) {
                for (size_t i = 0; i < C; i++) {
                    PyObject *column = PyList_GetItem(incoming, i);
                    toVec(column, outMatrix.column(i));
                }
            }
            else {
                throw("Passed PyObject pointer was not a list or tuple!");
            }
        }
    }

    /// @brief Convert to vector
    template<typename T, size_t N>
    void toVec(const py::object& incoming, Vector<T, N>& outVec) const {
        return toVec(incoming.ptr(), outVec);
    }
    template<typename T, size_t N>
    void toVec(PyObject* incoming, Vector<T, N>& outVec) const {
        if (isTuple(incoming)) {
            for (size_t i = 0; i < N; i++) {
                // PyTyple_GetItem does not call INCREF (unlike many functions that return an item)
                // So NO reference decrementing
                PyObject *value = PyTuple_GetItem(incoming, i);
                outVec[i] = cType<T>(value);
            }
        }
        else {
            if (isList(incoming)) {
                for (size_t i = 0; i < N; i++) {
                    PyObject *value = PyList_GetItem(incoming, i);
                    outVec[i] = cType<T>(value);
                }
            }
            else {
                throw("Passed PyObject pointer was not a list or tuple!");
            }
        }
    }

    template<typename T>
    std::vector<std::vector<T>> toVecOfVec(PyObject* incoming) const {
        std::vector<std::vector<T>> data;
        if (isTuple(incoming)) {
            Py_ssize_t size = PyTuple_Size(incoming);
            data.reserve(size);
            for (Py_ssize_t i = 0; i < size; i++) {
                // PyTyple_GetItem does not call INCREF (unlike many functions that return an item)
                // So NO reference decrementing
                PyObject *value = PyTuple_GetItem(incoming, i);
                Vec::EmplaceBack(data, toVec<T>(value));
            }
        }
        else {
            if (isList(incoming)) {
                Py_ssize_t size = PyList_Size(incoming);
                data.reserve(size);
                for (Py_ssize_t i = 0; i < size; i++) {
                    PyObject *value = PyList_GetItem(incoming, i);
                    Vec::EmplaceBack(data, toVec<T>(value));
                }
            }
            else {
                throw("Passed PyObject pointer was not a list or tuple!");
            }
        }
        return data;
    }

    /// @brief Convert a PyObject to a std vector
    template<typename T>
    std::vector<T> toVec(PyObject* incoming) const {
        std::vector<T> data;
        if (isTuple(incoming)) {
            Py_ssize_t size = PyTuple_Size(incoming);
            data.reserve(size);
            for (Py_ssize_t i = 0; i < size; i++) {
                // PyTyple_GetItem does not call INCREF (unlike many functions that return an item)
                // So NO reference decrementing
                PyObject *value = PyTuple_GetItem(incoming, i);
                Vec::EmplaceBack(data, cType<T>(value));
            }
        }
        else {
            if (isList(incoming)) {
                Py_ssize_t size = PyList_Size(incoming);
                data.reserve(size);
                for (Py_ssize_t i = 0; i < size; i++) {
                    PyObject *value = PyList_GetItem(incoming, i);
                    Vec::EmplaceBack(data, cType<T>(value));
                }
            }
            else {
                throw("Passed PyObject pointer was not a list or tuple!");
            }
        }
        return data;
    }
    template<typename T>
    std::vector<T> toVec(const py::object& incoming) const {
        return toVec<T>(incoming.ptr());
    }

    /// @brief Check if an object is a tuple
    bool isTuple(PyObject* o) const;
    bool isList(PyObject* o) const;
    bool isListLike(PyObject* o) const;
    bool isModule(PyObject* o) const;

    /// Create a python dict
    py::object toPyDict(const QJsonValue& json) const;

    /// Create a python double
    PyObject* toPyFloat(double d) const;

    /// @brief Create a python long
    PyObject* toPyLon(double d) const;
    PyObject* toPyLon(int i) const;

    /// @brief Create a python bool object
    PyObject* toPyBool(bool b) const;

    /// @brief Create a python string
    PyObject* toPyStr(const GString& s) const;

    /// @brief Convert to C type
    template<typename T>
    T cType(PyObject* o) const = delete;

    template <>
    GString cType<GString>(PyObject* o) const {
        PyObject* rep = PyObject_Repr(o);
        const char * s = PyUnicode_AsUTF8(rep);
        if (s == NULL) {
            printAndClearErrors();
            throw("Error, string conversion failed");
        }

        GString out(s);
        Py_XDECREF(rep);
        return out;
    }

    template<>
    double cType<double>(PyObject* o) const {
        return PyFloat_AsDouble(o);
    }

    template<>
    float cType<float>(PyObject* o) const {
        return PyFloat_AsDouble(o);
    }

    template<>
    int cType<int>(PyObject* o) const {
        return int(PyLong_AS_LONG(o));
    }

    /// @brief Print errors, returns true if errors were found
    QString printAndClearErrors() const;

    /// @brief Instantiate main module if it doesn't exist
    //void ensureMainModule();

    /// @brief Instantiate main module
    /// @details For importing packages, see:
    /// See: 
    /// https://stackoverflow.com/questions/28295674/define-python-class-from-c
    bool initializeMainModule();

    ///// @brief Get std::out contents
    //GString getStdOut() const;

    ///// @brief Get std::err contents
    //GString getStdErr() const;

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
    virtual const char* namespaceName() const { return "rev::PythonAPI"; }
    /// @}


protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    PythonAPI(const GString& programName = "python_gb");
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Initialize the Python interpreter with all required modules
    void initialize(const GString& programName);

    /// @brief Initialize python interpreter
    void initializeProgram(const GString& programName);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The python interpreter being used
    py::scoped_interpreter m_pythonInterpreter;

    /// @brief QString representing the python program
    wchar_t * m_programName;

    /// @brief Main module
    py::module_ m_main;

    /// @brief Sys module
    py::module_ m_sys;

    /// @brief Reverie module
    py::module_ m_reverie;

    py::module_ m_json;

    /// @brief Pointer to the only Singleton PythonAPI instance
    static std::unique_ptr<PythonAPI> s_instance;

    /// @brief Pointers to the std::out and std::err catchers
    PyObject* m_outCatcher;
    PyObject* m_errCatcher;

    /// @brief std::out and std::err messages to be logged
    GString m_stdOut;

    /// @brief Timer for logging
    QElapsedTimer m_timer;

    /// @}
};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif