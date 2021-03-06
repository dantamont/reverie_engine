/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_RENDER_CONTEXT_H
#define GB_RENDER_CONTEXT_H

// standard
#include <vector>

// QT
#include <QOpenGLBuffer>

// Internal
#include "../GGLFunctions.h"
#include "../buffers/GGLBuffer.h"
#include "../../GObject.h"
#include "../../mixins/GLoadable.h"
#include "GRenderSettings.h"

namespace rev {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class MainRenderer;
class LightingSettings;
class Texture;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RenderContext
/// @brief Context associated with a renderer
class RenderContext : public Object, public Serializable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    RenderContext(MainRenderer* renderer);
	~RenderContext();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

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

    //--------------------------------------------------------------------------------------------
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
    const Uuid& boundBuffer(GL::BufferType type) const {
        size_t idx = bufferTypeIndex(type);
        return m_boundBuffers[idx];
    }

    /// @brief Set the bound buffer of the given type
    void setBoundBuffer(GL::BufferType type, const Uuid& uuid) {
        size_t idx = bufferTypeIndex(type);
        m_boundBuffers[idx] = uuid;
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "RenderContext"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::RenderContext"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    ///// @brief Outputs this data as a valid json string
    //virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    ///// @brief Populates this data using a valid json string
    //virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

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

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Static methods
    /// @{

    static size_t bufferTypeIndex(const GL::BufferType& type);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The OpenGL context encapsulated by this object
    QOpenGLContext* m_context;
    QSurface* m_surface;

    /// @brief Light settings associated with this render context
    std::shared_ptr<LightingSettings> m_lightSettings;

    /// @brief The currently bound material's sort ID
    int m_boundMaterial = -1;

    /// @brief Currently bound buffer IDs, can currently be an SSB
    /// @details Are indexed by their buffer target type, e.g. UBO, SSB, etc.
    std::vector<Uuid> m_boundBuffers;

    /// @brief The currently bound OpenGL settings
    RenderSettings m_renderSettings;

    /// @brief a blank (white) 1x1 texture belonging to this context
    Texture* m_blankTexture;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Static members
    /// @{

    /// @brief Vector of types corresponding to indices of bound buffer
    static std::vector<GL::BufferType> s_bufferTypes;

    /// @}
};


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}

#endif