#pragma once

// QT
#include "geppetto/qt/widgets/graphics/GGLWidgetInterface.h"


namespace rev {
    class CoreEngine; 
    class RenderProjection;
	class OpenGlRenderer;
    class InputHandler;
}

namespace rev { 

class GLWidget : public GLWidgetInterface
{
    Q_OBJECT
public:

    GLWidget(const QString& name, rev::CoreEngine* engine, QWidget *parent);
    ~GLWidget();

    std::shared_ptr<OpenGlRenderer> renderer() { return m_renderer; }

    void addRenderProjection(RenderProjection* rp);

    /// @brief Reset widget (on engine clear)
    void clear() override;

    /// @brief Request a resize of the widget
    void requestResize() override;

public slots:
    /// @brief Update aspect ratios for all render projections
    void resizeProjections(int width, int height);
    void resizeProjections();

protected:

    friend class rev::InputHandler;

    /// @brief GL-specific overrides
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

    /// @brief set up signal/slot connections to render manager
    void initializeConnections();

    /// @}

	rev::CoreEngine* m_engine; //< core engine
	std::shared_ptr<OpenGlRenderer> m_renderer; ///< 3D rendering object
    std::vector<RenderProjection*> m_renderProjections; ///< Render projections associated with this widget


	/// @}
};

} // End rev

