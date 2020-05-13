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
#include "../containers/GbSortingLayer.h"
#include "../containers/GbContainerExtensions.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class ShaderProgram;
struct Uniform;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Class representing an object that can be rendered
class Renderable: public Serializable{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

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
    Renderable() {}
    virtual ~Renderable() {}
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    RenderSettings& renderSettings() { return m_renderSettings; }
    SortingLayer& renderLayer() { return m_renderLayer; }

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Add a uniform to the uniforms to be set
    void addUniform(const Uniform& uniform);

    /// @brief Draw the renderable given a shader program
    virtual void draw(const std::shared_ptr<ShaderProgram>& shaderProgram, RenderSettings* settings = nullptr);

    /// @brief Update any data needed for rendering, e.g. vertex data, render settings, etc.
    virtual void reload() = 0;

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

    /// @brief Set uniforms for the renderable in the given shader
    virtual void bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram);
    virtual void releaseUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram);

    /// @brief Bind the textures used by this renderable
    virtual void bindTextures() {}

    /// @brief Release the textures used by this renderable
    virtual void releaseTextures() {}

    /// @brief Draw geometry associated with this renderable
    virtual void drawGeometry(const std::shared_ptr<ShaderProgram>& shaderProgram, RenderSettings* settings = nullptr) {
        Q_UNUSED(settings);
        Q_UNUSED(shaderProgram);
    }

    /// @brief Print GL error
    void printError(const QString& error);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Uniforms corresponding to this renderable
    std::unordered_map<QString, Uniform> m_uniforms;

    /// @brief Render settings corresponding to this renderable
    RenderSettings m_renderSettings;

    /// @brief Rendering layer
    SortingLayer m_renderLayer;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif