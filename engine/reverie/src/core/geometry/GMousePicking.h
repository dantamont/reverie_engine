/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_MOUSE_PICKING_H
#define GB_MOUSE_PICKING_H

// Standard

// QT

// Internal
#include <core/geometry/GVector.h>

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class Camera;
class MainRenderer;
class Renderable;
class SceneObject;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @struct MouseHoverInfo
struct MouseHoverInfo {
    int m_sceneObjectId = -1;
    Renderable* m_renderablePtr = nullptr;
};

/// @class MousePicker
class MousePicker{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Convert an ID to a unique, three-channel color
    template<size_t N, typename OutType>
    static void GetChannelId(size_t id, OutType* outColor) {
        static_assert(N == 3 || N == 4, "Must have three or four channels");

        if constexpr (N == 3) {
            static size_t middleMask = 0x00FF00;
            static size_t leftMask = 0x0000FF;
            outColor[2] = OutType(id >> 16); // Shift 16 bits to get most significant bytes
            outColor[1] = OutType((middleMask & id) >> 8); // Shift 8 bits and mask to get middle bytes
            outColor[0] = OutType(leftMask & id);

            if (outColor[2] > 255) {
                throw("Error, more than 2^24 scene objects not supported");
            }
        }
        else {
            static size_t middleRightMask = 0x00FF0000;
            static size_t middleLeftMask = 0x0000FF00;
            static size_t leftMask = 0x000000FF;
            outColor[3] = OutType(id >> 24); // Shift 24 bits to get most significant bytes
            outColor[2] = OutType((middleRightMask & id) >> 16); // Shift 16 bits and mask to get middle bytes
            outColor[1] = OutType((middleLeftMask & id) >> 8); // Shift 8 bits and mask to get middle bytes
            outColor[0] = OutType(leftMask & id);

            if (outColor[3] > 255) {
                throw("Error, more than 2^32 scene objects not supported");
            }
        }
    }

    template<size_t N, typename OutType>
    static Vector<OutType, N> GetChannelId(size_t id) {
        Vector<OutType, N> channelId = Vector<OutType, N>::EmptyVector();
        GetChannelId<N, OutType>(id, channelId.array().data());
        return channelId;
    }

    /// @brief Convert a three-channel color to a one-dimensional Id
    template<size_t N, typename ColorType>
    static size_t GetColorId(ColorType* color) {
        static_assert(N == 3 || N == 4, "Must have three or four channels");

        if constexpr (N == 3) {
            return (color[2] << 16) + (color[1] << 8) + (color[0]);
        }
        else {
            return (color[3] << 24) + (color[2] << 16) + (color[1] << 8) + (color[0]);
        }
    }

    /// @}

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    MousePicker();
    virtual ~MousePicker();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief If the mouse is hovering over the scene object
    bool isMouseOver(SceneObject* sceneObject) const;

    /// @brief If the mouse is hovering over the renderable
    bool isMouseOver(Renderable* renderable) const;

    /// @brief Update the camera's pixelColor based on location of mouse cursor
    void updateMouseOver(const Vector2& widgetMousePos, const Camera& camera, const MainRenderer& renderer);


	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Get color at specified widget position for the specified camera
    void getPixelColor(Vector<int, 4>& outColor, const Vector3& widgetPos, const Camera& camera, const MainRenderer& renderer, size_t attachmentIndex, size_t sampleHalfWidth = 0);
    
    /// @brief Get color at specified widget position for the specified camera, as a float
    void getPixelColor(Vector4& outColor, const Vector3& widgetPos, const Camera& camera, const MainRenderer& renderer, size_t attachmentIndex, size_t sampleHalfWidth = 0);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The view-space normal that is moused over
    // TODO: Actually preserve this, the normal texture is getting cleared somewhere in the render pipeline
    //Vector4 m_mouseOverViewNormal;

    /// @brief The last mouse color hovered over
    /// @details Invalid if any channel is negative. Was using alpha channel, but fails if blending in color pass
    Vector4i m_mouseOverColor;

    /// @brief The current scene object and hoverable hovered over
    MouseHoverInfo m_hoverInfo;

    /// @}

};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif