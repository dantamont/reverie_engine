/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_PYTHON_SCRIPT_H
#define GB_PYTHON_SCRIPT_H

// External
#include <PythonQt.h>

// QT
#include <QString>

// Internal
#include "../GbObject.h"
#include "../mixins/GbLoadable.h"
#include "../containers/GbSortingLayer.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class PythonManager;
class SceneObject;

/////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Remove this class, is deprecated (does nothing useful)
/// @class PythonBehavior
/// @brief Class to get wrapped up by a python script to be called via python
class PythonBehavior: public Object {
public:

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    PythonBehavior();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{
    
    virtual void initialize();
    virtual void update(unsigned long deltaMs);
    virtual void fixedUpdate(unsigned long deltaMs);
    virtual void onSuccess();
    virtual void onFail();
    virtual void onAbort();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PythonBehavior"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PythonBehavior"; }
    /// @}

};
Q_DECLARE_METATYPE(PythonBehavior)

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PythonBehavior
/// @brief Represents a python behavior
class PythonBehaviorWrapper : public QObject, public Object {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Constructors/Destructor
    /// @{

    PythonBehaviorWrapper();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "PythonBehaviorWrapper"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::PythonBehaviorWrapper"; }
    /// @}

public slots:
    // Only slots are accessible through Python, so must map all desired routines

    /// @brief Add a constructor
    PythonBehavior* new_PythonBehavior();

    /// @brief Add a destructor
    void delete_PythonBehavior(PythonBehavior* o) { delete o; }

    /// @brief Script routines
    void initialize(PythonBehavior* o);
    void update(PythonBehavior* o, unsigned long deltaMs);
    void fixed_update(PythonBehavior* o, unsigned long deltaMs);
    void on_success(PythonBehavior* o);
    void on_fail(PythonBehavior* o);
    void on_abort(PythonBehavior* o);

private:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PythonScript
/// @brief Represents a python script
/// @note Filename must be the same as the class name defined in the script
class PythonScript : public Object, public Loadable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    PythonScript(CoreEngine* engine);
    PythonScript(CoreEngine* engine, const QString& filepath);
    virtual ~PythonScript();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Reload the script
    void reload();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Loadable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "PythonScript"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::PythonScript"; }
    /// @}


protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Get a python scene object, or create if there is none
    PyObject* getPythonSceneObject(const std::shared_ptr<SceneObject>& so) const;

    /// @brief Initialize the script
    virtual void initialize() = 0;

    /// @property Create in python
    virtual void defineInPython();


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief the core engine
    CoreEngine* m_engine;

    /// @brief Contents of the script
    QString m_contents;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class PythonClassScript
/// @brief Represents a python script
/// @note Filename must be the same as the class name defined in the script
class PythonClassScript : public PythonScript {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    PythonClassScript(CoreEngine* engine, const QJsonValue& json);
    PythonClassScript(CoreEngine* engine, const QString& filepath);
	~PythonClassScript();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property Sorting Layer
    SortingLayer* sortingLayer() { return m_sortingLayer; }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Instantiate this class on a scene object
    /// @note See: https://sourceforge.net/p/pythonqt/discussion/631393/thread/04acf1a9/
    /// https://sourceforge.net/p/pythonqt/discussion/631392/thread/3954953d/
    /// https://sourceforge.net/p/pythonqt/discussion/631393/thread/5890418f/
    PythonQtObjectPtr instantiate(const std::shared_ptr<SceneObject>& sceneObject);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Loadable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "PythonClassScript"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::PythonClassScript"; }
    /// @}


protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Initialize the script
    virtual void initialize() override;

    /// @property Create in python
    virtual void defineInPython() override;


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Name of class defined in python script
    QString m_className;

    /// @brief Sorting layer for the order of the script
    SortingLayer* m_sortingLayer = nullptr;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///// @class PythonListenerScript
///// @brief Represents a python script for responding to events
///// @note Filename must be the same as the class name defined in the script
//class PythonListenerScript : public PythonScript {
//public:
//    //--------------------------------------------------------------------------------------------
//    /// @name Static
//    /// @{
//    /// @}
//
//    //--------------------------------------------------------------------------------------------
//    /// @name Constructors/Destructor
//    /// @{
//    PythonListenerScript(CoreEngine* engine, const QJsonValue& json);
//    PythonListenerScript(CoreEngine* engine, const QString& filepath);
//    ~PythonListenerScript();
//    /// @}
//
//    //--------------------------------------------------------------------------------------------
//    /// @name Public Methods
//    /// @{
//
//    /// @brief Instantiate this class on a scene object
//    PythonQtObjectPtr instantiate(const std::shared_ptr<SceneObject>& sceneObject);
//
//    /// @}
//
//    //-----------------------------------------------------------------------------------------------------------------
//    /// @name Loadable Overrides
//    /// @{
//
//    /// @brief Outputs this data as a valid json string
//    QJsonValue asJson() const override;
//
//    /// @brief Populates this data using a valid json string
//    virtual void loadFromJson(const QJsonValue& json) override;
//
//    /// @}
//
//    //---------------------------------------------------------------------------------------
//    /// @name GB Object Properties 
//    /// @{
//
//    /// @property className
//    virtual const char* className() const { return "PythonListenerScript"; }
//
//    /// @property namespaceName
//    virtual const char* namespaceName() const { return "Gb::PythonListenerScript"; }
//    /// @}
//
//
//protected:
//
//    //--------------------------------------------------------------------------------------------
//    /// @name Protected Methods
//    /// @{
//
//    /// @brief Initialize the script
//    virtual void initialize() override;
//
//    /// @property Create in python
//    virtual void defineInPython() override;
//
//
//    /// @}
//
//    //--------------------------------------------------------------------------------------------
//    /// @name Protected Members
//    /// @{
//
//    /// @brief Name of the class defined in this script
//    QString m_className;
//
//    /// @}
//};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif