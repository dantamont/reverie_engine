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
class ResourceHandle;
class ShaderProgram;
class Shape;
class Mesh;
class Model;
class CubeMap;
class Material;
class ModelComponent;
class CanvasComponent;
class MainRenderer;


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
	~Renderer();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property Render Settings
    RenderSettings& renderSettings() { return m_renderSettings; }
    const RenderSettings& renderSettings() const { return m_renderSettings; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Uses specified shader to render
    //void draw(const std::vector<Renderable*>& renderables, const std::shared_ptr<ShaderProgram>& shaderProgram);

    /// @brief Add a uniform to set in the shader on rendering
    inline void addUniform(const Uniform& uniform) {
        m_uniforms[uniform.getName()] = uniform; 
    }
    
    /// @brief Clear all uniforms
    inline void clearUniforms() { m_uniforms.clear(); }

    virtual size_t getSortID() override { return 0; }

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

    friend class MainRenderer;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Set uniforms for the given shader
    /// @details Uniforms are pulled from the internal list, m_uniforms
    void bindUniforms(ShaderProgram& shaderProgram) override;
    void releaseUniforms(ShaderProgram& shaderProgram) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @}
};
Q_DECLARE_METATYPE(Renderer)

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}

#endif