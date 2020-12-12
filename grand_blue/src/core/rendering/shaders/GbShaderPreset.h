/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SHADER_PRESET_H
#define GB_SHADER_PRESET_H

// Standard
#include <memory>

// QT

// Internal
#include "../../GbObject.h"
#include "../../mixins/GbRenderable.h"
#include "../../containers/GbContainerExtensions.h"

namespace Gb {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class ShaderProgram;
class ShaderComponent;
//class GLBuffer;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class ShaderPreset
class ShaderPreset: public Object, public Shadable{
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

    const std::shared_ptr<ShaderProgram>& shaderProgram() const {
        return m_shaderProgram;
    }
    
    const std::shared_ptr<ShaderProgram>& prepassShaderProgram() const {
        return m_prepassShaderProgram;
    }

    void setShaderProgram(const std::shared_ptr<ShaderProgram>& sp) {
        m_shaderProgram = sp;
    }

    /// @}

	//---------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Add uniforms from this preset to the shader member's queue
    void queueUniforms();

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
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

    std::shared_ptr<ShaderProgram> m_shaderProgram;
    std::shared_ptr<ShaderProgram> m_prepassShaderProgram = nullptr;

    /// @brief Buffers associated with the preset
    //std::vector<GLBuffer*> m_buffers;

    /// @brief Vector of uniform names to preserve for GStringView
    std::vector<GString> m_uniformNames;

    /// @}
};
        
/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif