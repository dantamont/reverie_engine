//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_VIEWPORT_H
#define GB_VIEWPORT_H

// QT

// Internal
#include "../../mixins/GbLoadable.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////
class MainRenderer;
class FrameBuffer;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Settings for viewport
/// @details Currently 36 bytes
class Viewport{
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const;

    /// @brief Populates this data using a valid json string
    void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty());

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    int getDepth() const { return m_depth; }
    void setDepth(int depth);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{


    /// @brief Get depth in clip-space bounds [-1, 1]
    /// @details This routine is for setting FBO quad depth in quad shader
    float getClipDepth();

    /// @brief Set viewport in GL
    void setGLViewport(const MainRenderer& r) const;
    void setGLViewport(const FrameBuffer& fbo) const;
    void resizeFrameBuffer(size_t width, size_t height, FrameBuffer& fbo) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public members
    /// @{

    /// @brief Normal x-coordinate of camera on screen
    double m_xn = 0;

    /// @brief Normal y-coordinate of camera on screen
    double m_yn = 0;

    /// @brief Percent screen width of camera
    double m_width = 1;

    /// @brief Percent screen height of camera
    double m_height = 1;

    /// @}

private:

    /// @brief The order of viewport rendering, larger is rendered later (on top)
    int m_depth = 0;


    /// @brief Min/max depth of all viewports for normalization
    static std::array<int, 2> s_depthBounds;
};


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}


#endif