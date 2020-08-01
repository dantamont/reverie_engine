/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SHADER_PRESET_H
#define GB_SHADER_PRESET_H

// Standard
#include <memory>

// QT

// Internal
#include "../renderer/GbRenderers.h"
#include "../../containers/GbContainerExtensions.h"

namespace Gb {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class ShaderProgram;
class ShaderComponent;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class ShaderPreset
class ShaderPreset: public Object, public Serializable{
   
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ShaderPreset(CoreEngine* core, const QJsonValue& json);
    ShaderPreset(CoreEngine* core, const QString& name);
    ~ShaderPreset();
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const Renderer& renderer() const { return m_renderer; }
    Renderer& renderer() { return m_renderer; }

    const std::shared_ptr<ShaderProgram>& shaderProgram() const {
        return m_shaderProgram;
    }

    void setShaderProgram(const std::shared_ptr<ShaderProgram>& sp) {
        m_shaderProgram = sp;
    }

    /// @}

	//---------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
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
    virtual const char* className() const { return "ShaderPreset"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::GL::ShaderPreset"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    /// @}

protected:
    friend class ShaderComponent;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    CoreEngine* m_engine;
    Renderer m_renderer;

    std::shared_ptr<ShaderProgram> m_shaderProgram;

    /// @}
};
        
/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif