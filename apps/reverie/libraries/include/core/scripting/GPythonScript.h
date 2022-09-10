#pragma once

// External
#include "GPythonWrapper.h"

// QT
#include <QString>

// Internal
#include "fortress/types/GLoadable.h"
#include "fortress/containers/GSortingLayer.h"
#include "core/resource/GResourceHandle.h"

namespace py = pybind11;
namespace rev {

class CoreEngine;
class PythonManager;
class SceneObject;

/// @class PythonScript
/// @brief Represents a python script
/// @note Filename must be the same as the class name defined in the script
class PythonScript : public Resource, public LoadableInterface {
public:

    /// @name Constructors/Destructor
    /// @{
    PythonScript(CoreEngine* engine);
    PythonScript(CoreEngine* engine, const QString& filepath);
    virtual ~PythonScript();
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Get the type of resource stored by this handle
    virtual GResourceType getResourceType() const override {
        return EResourceType::ePythonScript;
    }

    /// @brief Reload the script
    void reload();

    /// @}

    /// @name LoadableInterface Overrides
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const PythonScript& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, PythonScript& orObject);


    /// @}

protected:

    /// @name Protected Methods
    /// @{

    /// @brief Get a python scene object, or create if there is none
    py::object getPythonSceneObject(const std::shared_ptr<SceneObject>& so) const;

    /// @brief Initialize the script
    virtual void initialize() = 0;

    /// @property Create in python
    virtual void defineInPython();


    /// @}

    /// @name Protected Members
    /// @{

    /// @brief the core engine
    CoreEngine* m_engine;

    /// @brief Contents of the script
    QString m_contents;

    /// @}
};


/// @class PythonClassScript
/// @brief Represents a python script
/// @note Filename must be the same as the class name defined in the script
class PythonClassScript : public PythonScript {
public:

	/// @name Constructors/Destructor
	/// @{
    PythonClassScript(CoreEngine* engine, const nlohmann::json& json);
    PythonClassScript(CoreEngine* engine, const QString& filepath);
	~PythonClassScript();
	/// @}

    /// @name Properties
    /// @{

    const QString& getClassName() const { return m_className; }

    /// @}

	/// @name Public Methods
	/// @{

    /// @brief Instantiate PythonClassScript on a scene object
    /// @note See: https://sourceforge.net/p/pythonqt/discussion/631393/thread/04acf1a9/
    /// https://sourceforge.net/p/pythonqt/discussion/631392/thread/3954953d/
    /// https://sourceforge.net/p/pythonqt/discussion/631393/thread/5890418f/
    py::object instantiate(const std::shared_ptr<SceneObject>& sceneObject);

    /// @brief What action to perform on removal of the resource
    virtual void onRemoval(ResourceCache* /*cache*/ = nullptr) override;

    Uint32_t sortingLayer() const {
        return m_sortingLayer;
    }

    /// @}


    /// @name Friends
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const PythonClassScript& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, PythonClassScript& orObject);


    /// @}

protected:

    /// @name Protected Methods
    /// @{

    /// @brief Initialize the script
    virtual void initialize() override;

    /// @property Create in python
    virtual void defineInPython() override;


    /// @}

    /// @name Protected Members
    /// @{

    static constexpr Uint32_t s_invalidSortingLayerValue = std::numeric_limits<Uint32_t>::max();
    QString m_className; ///< Name of class defined in python script
    Uint32_t m_sortingLayer = s_invalidSortingLayerValue; ///< Sorting layer ID for the order of the script

    /// @}
};


} // End namespaces
