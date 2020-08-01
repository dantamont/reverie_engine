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
#include "../GbGLFunctions.h"
#include "../../GbObject.h"
#include "../../mixins/GbLoadable.h"

namespace Gb {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class MainRenderer;
class LightingSettings;

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

    LightingSettings& lightingSettings() { return *m_lightSettings; }

    /// @brief Obtain underlying context
    QOpenGLContext* context() { return m_context; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    bool isCurrent() const;

    void makeCurrent();

    const Uuid& boundBuffer() const {
        return m_boundBuffer;
    }
    void setBoundBuffer(const Uuid& uuid) {
        m_boundBuffer = uuid;
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "RenderContext"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::RenderContext"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    ///// @brief Outputs this data as a valid json string
    //virtual QJsonValue asJson() const override;

    ///// @brief Populates this data using a valid json string
    //virtual void loadFromJson(const QJsonValue& json) override;

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
    /// @name Protected members
    /// @{

    /// @brief The OpenGL context encapsulated by this object
    QOpenGLContext* m_context;
    QSurface* m_surface;

    /// @brief Light settings associated with this render context
    std::shared_ptr<LightingSettings> m_lightSettings;

    /// @brief Currently bound buffer ID, can currently be an SSB
    Uuid m_boundBuffer;

    /// @}
};


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}

#endif