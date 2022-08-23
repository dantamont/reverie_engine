#pragma once

// Standard
#include <memory>

// QT

// Internal
#include "core/mixins/GRenderable.h"
#include "fortress/containers/GContainerExtensions.h"

namespace rev {  

class UniformContainer;
class CoreEngine;
class DrawCommand;
class ShaderProgram;
class ShaderComponent;

/// @class ShaderPreset
class ShaderPreset: public NameableInterface, public IdentifiableInterface {
public:
    /// @name Static
    /// @{

    /// @brief Obtain a built-in preset by name
    static std::shared_ptr<ShaderPreset> GetBuiltin(const GString& name);

    /// @brief Clear builtin presets
    static void InitializeBuiltins(CoreEngine* engine);

    /// @}

    /// @name Constructors/Destructor
    /// @{
    ShaderPreset(CoreEngine* core, const nlohmann::json& json);
    ShaderPreset(CoreEngine* core, const GString& name);
    ~ShaderPreset();
    /// @}

    /// @name Public Methods
    /// @{

    CoreEngine* engine() { return m_engine; }

    ShaderProgram* shaderProgram() const {
        return m_shaderProgram;
    }
    
    ShaderProgram* prepassShaderProgram() const {
        return m_prepassShaderProgram;
    }

    void setShaderProgram(ShaderProgram* sp) {
        m_shaderProgram = sp;
    }

    /// @brief Add uniforms from this preset to the shader member's queue
    void queueUniforms();

    /// @brief Apply preset to a draw command
    /// @details This adds the uniforms and sets the render settings from the preset to/for the draw command
    void applyPreset(DrawCommand& command) const;

    const RenderSettings& renderSettings() const { return m_renderSettings; }
    void setRenderSettings(const RenderSettings& settings) { m_renderSettings = settings; }

    /// @brief Add a uniform to the uniforms to be set
    template<typename ValueType>
    void addUniform(Uint32_t id, const ValueType& value) {
        int idx = -1;
        if (hasUniform(id, &idx)) {
            Uniform& uniform = m_uniforms[idx];
            uniform.setValue(value, m_uniformValues);
        }
        else {
            // Main shader and prepass shader uniform point to the same value in the uniform container
            m_uniforms.emplace_back(id, value, m_uniformValues);
            if (m_prepassShaderProgram) {
                const GStringView& uniformName = m_uniforms.back().getName(*m_shaderProgram);
                Int32_t prepassId{ -1 };
                if (m_prepassShaderProgram->hasUniform(uniformName)) {
                    prepassId = m_prepassShaderProgram->getUniformId(uniformName);
                }
                m_prepassUniformIds.emplace_back(prepassId);
            }
        }
    }
    void addUniform(const Uniform& uniform);

    /// @brief Whether or not the command has the specified uniform
    bool hasUniform(Uint32_t uniformId, int* outIndex = nullptr);
    bool hasUniform(const Uniform& uniform, int* outIndex = nullptr);

    /// @brief Clear all uniforms
    void clearUniforms();

	/// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const ShaderPreset& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, ShaderPreset& orObject);


    /// @}

protected:
    friend class ShaderComponent;

    /// @name Protected members
    /// @{

    std::vector<Uniform> m_uniforms; ///< Uniforms corresponding to this preset
    UniformContainer& m_uniformValues; ///< The uniform values corresponding to this preset

    RenderSettings m_renderSettings; ///< GL Render settings corresponding to this preset

    CoreEngine* m_engine{ nullptr };
    ShaderProgram* m_shaderProgram{ nullptr };
    ShaderProgram* m_prepassShaderProgram{ nullptr };
    std::vector<Int32_t> m_prepassUniformIds; ///< The IDs for each uniform in the prepass shader. 1:1 with m_uniforms list
    static std::vector<std::shared_ptr<ShaderPreset>> s_builtins; ///< List of built-in shader presets

    /// @}
};
        

} // End namespaces
