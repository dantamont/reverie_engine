/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_RENDERABLE_H
#define GB_RENDERABLE_H

// Standard
#include <map>

// QT
#include <QString>

// Internal
#include "GbLoadable.h"
#include "../rendering/renderer/GbRenderSettings.h"
#include "../containers/GbContainerExtensions.h"
#include "../geometry/GbCollisions.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class ShaderProgram;
struct Uniform;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class Shadable
/// @brief Class representing an object that carries uniforms
class Shadable : public Serializable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum TransparencyType {
        kOpaque = 0,
        kTransparentNormal,
        kTransparentAdditive, // Adds colors together, so result will be brighter than either
        kTransparentSubtractive // Darkens colors behind the material by subtracting the material's colors from the background colors
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Shadable() {}
    virtual ~Shadable() {}
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const std::unordered_map<QString, Uniform>& uniforms() const { return m_uniforms; }
    std::unordered_map<QString, Uniform>& uniforms() { return m_uniforms; }

    RenderSettings& renderSettings() { return m_renderSettings; }

    const TransparencyType& transparencyType() const { return m_transparencyType; }
    TransparencyType& transparencyType() { return m_transparencyType; }
    void setTransparencyType(const TransparencyType& type) {
        m_transparencyType = type;
    }

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Add a uniform to the uniforms to be set
    void addUniform(const Uniform& uniform);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Uniforms corresponding to this renderable
    std::unordered_map<QString, Uniform> m_uniforms;

    /// @brief GL Render settings corresponding to this renderable
    RenderSettings m_renderSettings;

    TransparencyType m_transparencyType = kOpaque;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class Renderable
/// @brief Class representing an object that generates a draw item
class Renderable: public Shadable{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Flags relating to render passes
    enum RenderPassFlag {
        kIgnoreSettings = 1 << 0, // avoid binding render settings
        kIgnoreTextures = 1 << 1, // skip texture binding
        kIgnoreUniforms = 1 << 2, // skip uniform binding
    };

    static QSize screenDimensions();
    static Vector2g screenDimensionsVec();

    /// @brief Return screen width and height in pixels
    static float screenDPI();
    static float screenDPIX();
    static float screenDPIY();

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Renderable();
    virtual ~Renderable() {}
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief The bounding geometry for this renderable
    BoundingBoxes& boundingBoxes() {
        return m_bounds;
    }

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Draw the renderable given a shader program
    virtual void draw(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr, size_t drawFlags = 0);
    //virtual void draw(RenderCommand& command);

    /// @brief Update any data needed for rendering, e.g. vertex data, render settings, etc.
    virtual void reload() {}

    /// @brief Get ID to sort renderable
    virtual size_t getSortID() = 0;

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Miscellaneous actions to be performed before drawing
    /// @details Is a good place for any viewport changes to be made
    virtual void preDraw(){}

    /// @brief Set uniforms for the renderable in the given shader
    virtual void bindUniforms(ShaderProgram& shaderProgram);
    virtual void releaseUniforms(ShaderProgram& shaderProgram);

    /// @brief Bind the textures used by this renderable
    virtual void bindTextures(ShaderProgram* shaderProgram = nullptr) 
    {
        Q_UNUSED(shaderProgram);
    }

    /// @brief Release the textures used by this renderable
    virtual void releaseTextures(ShaderProgram* shaderProgram = nullptr)
    {
        Q_UNUSED(shaderProgram);
    }

    /// @brief Draw geometry associated with this renderable
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) 
    {
        Q_UNUSED(settings);
        Q_UNUSED(shaderProgram);
    }

    /// @brief Print GL error
    void printError(const QString& error);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The bounding geometry for this renderable
    // TODO: Maybe make this a simple AABB, but having multiple bounding boxes makes bounding more accurate
    // even if the bounding boxes are treates as a single unit in terms of the rendering check
    BoundingBoxes m_bounds;

    /// @}

};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif