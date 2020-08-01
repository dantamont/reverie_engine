#ifndef GB_CUBEMAP_COMPONENT
#define GB_CUBEMAP_COMPONENT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes
#include <memory>

// Qt
#include <QString>
#include <QJsonValue>

// Project
#include "GbComponent.h"
#include "../mixins/GbRenderable.h"
#include "../resource/GbResource.h"
#include "../containers/GbColor.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CubeMap;
class CubeTexture;
class ShaderProgram;
class RenderSettings;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @class CubeMapComponent
/// @brief Class representing a CubeMap 
/// @details Sampled in shader as a 3D direction vector
/// @note See: https://learnopengl.com/Advanced-OpenGL/Cubemaps
class CubeMapComponent: public Component, 
    public Renderable, 
    protected GL::OpenGLFunctions {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static Methods
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    CubeMapComponent();
    CubeMapComponent(const CubeMapComponent & component);
    CubeMapComponent(const std::shared_ptr<SceneObject>& object, const QJsonValue& json);
    CubeMapComponent(const std::shared_ptr<SceneObject>& object);
    ~CubeMapComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Enable the behavior of this script component
    virtual void enable() override;

    /// @brief Disable the behavior of this script component
    virtual void disable() override;

    /// @brief Max number of allowed components per scene
    virtual int maxAllowed() const override { return 1; }

    /// @brief Set as default cubemap for the scene
    void setDefault();
    bool isDefault() const;

    /// @property geometry for this model
    void draw(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr, size_t drawFlags = 0) override;

    /// @brief Set the cube texture at the given filepath, loading if required
    void setCubeTexture(const QString& filepath);

    virtual void reload() override {}

    std::shared_ptr<CubeTexture> texture();

    virtual size_t getSortID() override { return 0; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Diffuse color
    Color& diffuseColor() { return m_color; }
    void setDiffuseColor(const Color& color) {
        m_color = color;
    }

    const std::shared_ptr<ResourceHandle>& textureHandle() const { return m_cubeTextureHandle; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "CubeMapComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::CubeMapComponent"; }

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
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief checkValidity the cubemap
    void initialize();

    /// @brief Set uniforms in the shader being used for rendering
    void bindUniforms(ShaderProgram& shaderProgram, const std::shared_ptr<CubeTexture>& cubeTexture);


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The texture corresponding to this cubemap
    std::shared_ptr<ResourceHandle> m_cubeTextureHandle;

    /// @brief The cube mesh, stored for convenience;
    std::shared_ptr<Mesh> m_cubeMesh;

    /// @brief The diffuse color of the cubemap
    Color m_color;

    /// @}

};
Q_DECLARE_METATYPE(CubeMapComponent)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
