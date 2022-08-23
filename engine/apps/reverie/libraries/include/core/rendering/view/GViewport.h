#pragma once

// Internal
#include "fortress/containers/math/GVector.h"

namespace rev {

class OpenGlRenderer;
class FrameBuffer;


/// @brief Settings for viewport
/// @details Currently 36 bytes
class Viewport{
public:

    /// @name Static
    /// @{

    static Vector2i ScreenDimensions();

    /// @brief Return screen width and height in pixels
    static float ScreenDPI();
    static float ScreenDPIX();
    static float ScreenDPIY();

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Viewport& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Viewport& orObject);

    /// @}

    /// @name Properties
    /// @{

    int getDepth() const { return m_depth; }
    void setDepth(int depth);

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Check if mouse is in bounds
    bool inBounds(const Vector2& widgetPos, const OpenGlRenderer& renderer, float & outClipX, float & outClipY) const;

    /// @brief Convert from widget (device) coordinates to homogeneous clip coordinates
    /// @details If given a z component (-1 to 1, mapped from depth buffer [0,1] via (2.0 * depth - 1.0))
    /// with w = 1.0, the clip coordinates can then be converted to world coordinates
    /// by multiplying by the inverse projection/view matrix and performing a perspective divide
    /// @param[in] widgetSpace 2D pixel coordinates within the render target, with (0, 0) in the upper-left corner, X = right, Y = down
    /// @note See: https://computergraphics.stackexchange.com/questions/1769/world-coordinates-normalised-device-coordinates-and-device-coordinates
    void widgetToClip(const Vector2& widgetSpace, const OpenGlRenderer& renderer, float& outClipX, float& outClipY) const;

    /// @brief Get depth in clip-space bounds [-1, 1]
    /// @details This routine is for setting FBO quad depth in quad shader
    float getClipDepth();

    /// @brief Set viewport in GL
    void setGLViewport(const OpenGlRenderer& r) const;
    void setGLViewport(const FrameBuffer& fbo) const;
    void resizeFrameBuffer(uint32_t width, uint32_t height, FrameBuffer& fbo) const;

    /// @}
  
    /// @name Public members
    /// @{

    double m_xn = 0; ///< Normalized x-coordinate of camera on screen [-1, 1]
    double m_yn = 0; ///< Normalized y-coordinate of camera on screen [-1, 1]
    double m_width = 1; ///< Percent screen width of camera
    double m_height = 1; ///< Percent screen height of camera

    /// @}

private:

    int m_depth = 0; ///< The order of viewport rendering, larger is rendered later (on top)
    static std::array<int, 2> s_depthBounds; ///< Min/max depth of all viewports for normalization
};

// End namespaces
}
