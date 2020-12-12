#ifndef GB_CANVAS_COMPONENT
#define GB_CANVAS_COMPONENT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt
#include <QString>
#include <QJsonValue>
#include <QSize>

// Project
#include "GbCameraComponent.h"
#include "../mixins/GbRenderable.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Uniform;
class ModelComponent;
class ShaderProgram;
class Label;
class Icon;
class Glyph;
class CameraComponent;
class DrawCommand;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @class CanvasComponent
class CanvasComponent: public Component, public Shadable {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static Methods
    /// @{

    /// @brief Identity matrix
    static const Matrix4x4g& ViewMatrix() { return VIEW_MATRIX; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    CanvasComponent();
    CanvasComponent(CoreEngine* core);
    CanvasComponent(const std::shared_ptr<SceneObject>& object);
    ~CanvasComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Create the draw commands for the canvas component
    void createDrawCommands(
        std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands,
        SceneCamera& camera,
        ShaderProgram& shaderProgram,
        ShaderProgram* prepassProgram = nullptr);

    void addGlyph(const std::shared_ptr<Glyph>& glyph);
    void removeGlyph(const Glyph& glyph);
    void removeGlyph(const std::shared_ptr<Glyph>& glyph);

    QSize mainGLWidgetDimensions() const;

    /// @brief Resize viewport for the canvas component
    void resizeViewport();

    /// @brief Clear the canvas
    void clear();

    ///// @brief Draw the canvas with the given shader program
    //void draw(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @brief Enable the behavior of this script component
    virtual void enable() override;

    /// @brief Disable the behavior of this script component
    virtual void disable() override;

    /// @brief Max number of allowed components per scene object
    /// @note Limiting to one because it would be a poor design to allow more than one canvas per object
    virtual int maxAllowed() const override { return 1; }

    virtual void addRequiredComponents() override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const std::vector<std::shared_ptr<Glyph>>& glyphs() const { return m_glyphs; }

    /// @brief Viewport settings
    Viewport& viewport() { return m_viewport; }

    /// @brief Holds information about projection
    RenderProjection& renderProjection() { return m_renderProjection; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "CanvasComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::CanvasComponent"; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Reload all glyphs if screen dimensions changed
    void resize();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Most recently cached widget dimensions
    QSize m_dimensions;

    /// @brief Viewport settings
    // TODO: See if this is deprecated, with rendering to every camera
    Viewport m_viewport;

    /// @brief Holds information about projection
    RenderProjection m_renderProjection;

    /// @brief All of the glyphs on this canvas
    std::vector<std::shared_ptr<Glyph>> m_glyphs;

    /// @brief Should always be identity
    static Matrix4x4g VIEW_MATRIX;

    /// @}


};
Q_DECLARE_METATYPE(CanvasComponent)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 
