//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_VIEWPORT_H
#define GB_VIEWPORT_H

// QT

// Internal
#include "../../mixins/GLoadable.h"
#include <core/geometry/GVector.h>

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef GB_USE_DOUBLE
typedef double real_g;
#else
typedef float real_g;
#endif

class MainRenderer;
class FrameBuffer;
//template <typename D, size_t size> class Vector;
//typedef Vector<real_g, 2> Vector2;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Settings for viewport
/// @details Currently 36 bytes
class Viewport{
public:

    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    static QSize ScreenDimensions();
    static Vector2 ScreenDimensionsVec();

    /// @brief Return screen width and height in pixels
    static float ScreenDPI();
    static float ScreenDPIX();
    static float ScreenDPIY();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const;

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

    /// @brief Check if mouse is in bounds
    bool inBounds(const Vector2& widgetPos, const MainRenderer& renderer, float & outClipX, float & outClipY) const;

    /// @brief Convert from widget (device) coordinates to homogeneous clip coordinates
    /// @details If given a z component (-1 to 1, mapped from depth buffer [0,1] via (2.0 * depth - 1.0))
    /// with w = 1.0, the clip coordinates can then be converted to world coordinates
    /// by multiplying by the inverse projection/view matrix and performing a perspective divide
    /// @param[in] widgetSpace 2D pixel coordinates within the render target, with (0, 0) in the upper-left corner, X = right, Y = down
    /// @note See: https://computergraphics.stackexchange.com/questions/1769/world-coordinates-normalised-device-coordinates-and-device-coordinates
    void widgetToClip(const Vector2& widgetSpace, const MainRenderer& renderer, float& outClipX, float& outClipY) const;


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

    /// @brief Normalized x-coordinate of camera on screen [-1, 1]
    /// @details Coordinate is center of viewport
    double m_xn = 0;

    /// @brief Normalized y-coordinate of camera on screen [-1, 1]
        /// @details Coordinate is center of viewport
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