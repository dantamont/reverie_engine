/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_RENDERERS_H
#define GB_RENDERERS_H

// QT
#include <QOpenGLBuffer>

// Internal
#include "../../components/GbComponent.h"
#include "../GbGLFunctions.h"
#include "../shaders/GbUniform.h"
#include "GbRenderSettings.h"
#include "../../mixins/GbRenderable.h"

namespace Gb {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
struct VertexAttributes;
class CoreEngine;
class ResourceHandle;
class ShaderProgram;
class Shape;
class Mesh;
class Model;
class CubeMap;
class Material;
class ModelComponent;
class CanvasComponent;

namespace GL {
class MainRenderer;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class Renderer
/// @brief For rendering 3D geometry
/// @detailed Corresponds to a particular shader
class Renderer : public Object, public Renderable, protected GL::OpenGLFunctions {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Renderer();
	Renderer(CoreEngine* engine);
	~Renderer();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property Render Settings
    RenderSettings& renderSettings() { return m_renderSettings; }

    /// @property Shader Program
    /// @brief The shader to be used to render geometry
    const std::shared_ptr<ShaderProgram>& shaderProgram() { return m_shaderProgram; }
    inline void setShaderProgram(const std::shared_ptr<ShaderProgram>& shaderProgram) { m_shaderProgram = shaderProgram; }

    /// @brief Return render layer
    inline const SortingLayer& getRenderLayer() const { return m_renderLayer; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Render a canvas
    void draw(const std::vector<Renderable*>& renderables);

    /// @brief Add a uniform to set in the shader on rendering
    inline void addUniform(const Uniform& uniform) {
        m_uniforms[uniform.getName()] = uniform; 
    }
    
    /// @brief Clear all uniforms
    inline void clearUniforms() { m_uniforms.clear(); }

    /// @brief Renderable override
    virtual void reload() override {}

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "Renderer"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::Renderer"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class GL::MainRenderer;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Uses specified shader to render
    void draw(const std::vector<Renderable*>& renderables, const std::shared_ptr<ShaderProgram>& shaderProgram);

    /// @brief Set uniforms for the given shader
    /// @details Uniforms are pulled from the internal list, m_uniforms
    void bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram) override;
    void releaseUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Pointer to the core engine
    CoreEngine* m_engine;

    /// @brief Shader to be used to render geometry by default
    std::shared_ptr<ShaderProgram> m_shaderProgram;

    /// @}
};
Q_DECLARE_METATYPE(Renderer)

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}

#endif