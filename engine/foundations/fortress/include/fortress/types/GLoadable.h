/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LOADABLE_H
#define GB_LOADABLE_H

// Standard
#include <vector>

// Internal
#include "fortress/types/GString.h"
#include <fortress/json/GJson.h>

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class CoreEngine;


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class representing an object that can be loaded and saved from a file
class LoadableInterface {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Formats that data can be loaded from
    enum Format {
        kJson,
        kBinary // Load directly into memory
    };

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    LoadableInterface(const GString& filepath):
        m_path(filepath) {}
    LoadableInterface() {}

	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property File or directory path
    const GString& getPath() const { return m_path; }
    void setPath(const GString& path) { m_path = path; }

    /// @}

    /// @name Friend methods
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson 
    /// @param korObject
    friend void to_json(nlohmann::json& orJson, const LoadableInterface& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson 
    /// @param orObject 
    friend void from_json(const nlohmann::json& korJson, LoadableInterface& orObject);

    /// @}

protected:

    /// @brief Destructor made protected to prevent polymorphic deletion of non-virtual destructor
    ~LoadableInterface() {}

    GString m_path; ///< Filepath to the object

};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @todo Use composition instead of inheritance
/// @brief Class representing an object that can be loaded and saved from/to multiple files
class DistributedLoadableInterface : public LoadableInterface {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    explicit DistributedLoadableInterface(const LoadableInterface& loadable):
        LoadableInterface(loadable.getPath())
    {
    }
    DistributedLoadableInterface(const GString& filepath): LoadableInterface(filepath){
    }
    DistributedLoadableInterface() {
    }
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    std::vector<GString>& additionalPaths() { return m_additionalPaths; }

    /// @}

    /// @name Friend methods
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson 
    /// @param korObject
    friend void to_json(nlohmann::json& orJson, const DistributedLoadableInterface& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson 
    /// @param orObject 
    friend void from_json(const nlohmann::json& korJson, DistributedLoadableInterface& orObject);

    /// @}

protected:

    /// @brief Destructor made protected to prevent polymorphic deletion of non-virtual destructor
    ~DistributedLoadableInterface() {}


    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Additional paths for the loadable
    std::vector<GString> m_additionalPaths;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif