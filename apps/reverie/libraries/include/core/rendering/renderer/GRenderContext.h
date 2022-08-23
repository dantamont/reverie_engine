#pragma once

// standard
#include <vector>

// QT
#include <QOpenGLBuffer>

// Internal
#include "core/rendering/GGLFunctions.h"
#include "core/rendering/buffers/GGlBuffer.h"
#include "core/rendering/shaders/GUniformContainer.h"
#include "fortress/types/GLoadable.h"
#include "GRenderSettings.h"

namespace rev {  

class OpenGlRenderer;
class LightingSettings;
class Texture;

/// @class RenderContext
/// @brief Context associated with a renderer
class RenderContext {
public:
	/// @name Constructors/Destructor
	/// @{
    RenderContext(QOpenGLContext* context, QSurface* surface);
	~RenderContext();
	/// @}

    /// @name Properties
    /// @{

    UniformContainer& uniformContainer() {
        return m_uniformContainer;
    }

    Texture& blankTexture() const { return *m_blankTexture; }

    LightingSettings& lightingSettings() { return *m_lightSettings; }

    /// @brief Obtain underlying context
    QOpenGLContext* context() { return m_context; }

    int boundMaterial() const { return m_boundMaterial; }
    void setBoundMaterial(int idx) { m_boundMaterial = idx; }

    RenderSettings& renderSettings() { return m_renderSettings; }

    /// @brief Get GL functions from current contect
    inline QOpenGLFunctions* functions() {
        return m_context->functions();
    }

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Reset settings for a new scenario load
    void reset();

    /// @brief Flush all buffer data into OpenGL
    void flushBuffers();

    /// @brief Swap all buffers for read/write
    void swapBuffers();

    /// @brief Method for configuring an OpenGL setting
    template<typename T, typename ...Args>
    inline void flushSetting(Args&&... args) {
        // Create the setting, adding it to current settings
        const T& setting = static_cast<const T&>(
            m_renderSettings.addSetting<T>(std::forward<Args>(args)...));
        //std::shared_ptr<RenderSetting> setting = std::make_shared<T> (std::forward<Args>(args)...);
        
        // Set the actual settings in OpenGL, without worrying about caching previous values
        setting.set(*this);
    }

    bool isCurrent() const;

    void makeCurrent();

    /// @brief Get the bound buffer of the given type
    const Uuid& boundBuffer(gl::BufferType type) const {
        size_t idx = bufferTypeIndex(type);
        return m_boundBuffers[idx];
    }

    /// @brief Set the bound buffer of the given type
    void setBoundBuffer(gl::BufferType type, const Uuid& uuid) {
        size_t idx = bufferTypeIndex(type);
        m_boundBuffers[idx] = uuid;
    }

    /// @}

protected:
    /// @name Friends
    /// @{

    friend class OpenGlRenderer;

    /// @}

    /// @name Protected methods
    /// @{

    void toggleDebugOutput();

    void resetLights();

    /// @}

    /// @name Static methods
    /// @{

    static size_t bufferTypeIndex(const gl::BufferType& type);

    /// @}

    /// @name Protected members
    /// @{

    UniformContainer m_uniformContainer; ///< Uniform values for the render context
    QOpenGLContext* m_context{ nullptr }; ///< The OpenGL context encapsulated by this object
    QSurface* m_surface{ nullptr }; ///< Qt's abstraction of a renderable surface
    std::unique_ptr<LightingSettings> m_lightSettings{ nullptr }; ///< Light settings associated with this render context
    int m_boundMaterial = -1; ///< The currently bound material's sort ID
    RenderSettings m_renderSettings; ///< The currently bound OpenGL settings
    Texture* m_blankTexture; ///< A blank (white) 1x1 texture belonging to this context

    /// @details Are indexed by their buffer target type, e.g. UBO, SSB, etc.
    std::vector<Uuid> m_boundBuffers; ///< Currently bound buffer IDs, can currently be an SSB

    /// @}

    /// @name Static members
    /// @{

    static std::vector<gl::BufferType> s_bufferTypes; ///< Vector of types corresponding to indices of bound buffer

    /// @}
};


    
// End namespaces
}
