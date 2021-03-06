/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SHADER_PRESET_H
#define GB_SHADER_PRESET_H

// Standard
#include <memory>

// QT

// Internal
#include "../../GObject.h"
#include "../../mixins/GRenderable.h"
#include "../../containers/GContainerExtensions.h"

namespace rev {  

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
class ShaderPreset: public Object, public Nameable, public Identifiable, public Shadable{
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Obtain a built-in preset by name
    static std::shared_ptr<ShaderPreset> GetBuiltin(const GString& name);

    /// @brief Clear builtin presets
    static void InitializeBuiltins(CoreEngine* engine);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ShaderPreset(CoreEngine* core, const QJsonValue& json);
    ShaderPreset(CoreEngine* core, const GString& name);
    ~ShaderPreset();
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    ShaderProgram* shaderProgram() const {
        return m_shaderProgram;
    }
    
    ShaderProgram* prepassShaderProgram() const {
        return m_prepassShaderProgram;
    }

    void setShaderProgram(ShaderProgram* sp) {
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
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties
    /// @{
    /// @property className
    virtual const char* className() const { return "ShaderPreset"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::GL::ShaderPreset"; }
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

    ShaderProgram* m_shaderProgram;
    ShaderProgram* m_prepassShaderProgram = nullptr;

    /// @brief Buffers associated with the preset
    //std::vector<GLBuffer*> m_buffers;

    /// @brief Vector of uniform names to preserve for GStringView
    std::vector<GString> m_uniformNames;

    /// @brief List of built-in shader presets
    static std::vector<std::shared_ptr<ShaderPreset>> s_builtins;

    /// @}
};
        
/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif