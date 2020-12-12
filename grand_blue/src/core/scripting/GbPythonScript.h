/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_PYTHON_SCRIPT_H
#define GB_PYTHON_SCRIPT_H

// External
#include "GbPythonWrapper.h"

// QT
#include <QString>

// Internal
#include "../GbObject.h"
#include "../mixins/GbLoadable.h"
#include "../containers/GbSortingLayer.h"
#include "../resource/GbResource.h"

namespace py = pybind11;
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
/// @class PythonScript
/// @brief Represents a python script
/// @note Filename must be the same as the class name defined in the script
class PythonScript : public Resource, public Loadable {
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

    /// @brief Get the type of resource stored by this handle
    virtual Resource::ResourceType getResourceType() const override {
        return Resource::kPythonScript;
    }

    /// @brief Reload the script
    void reload();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Loadable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

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
    py::object getPythonSceneObject(const std::shared_ptr<SceneObject>& so) const;

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

    const QString& getClassName() const { return m_className; }


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
    py::object instantiate(const std::shared_ptr<SceneObject>& sceneObject);

    /// @brief What action to perform on removal of the resource
    virtual void onRemoval(ResourceCache* cache = nullptr) override {
        Q_UNUSED(cache)
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Loadable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

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


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif