#ifndef GB_RENDERABLE_WIDGET_H 
#define GB_RENDERABLE_WIDGET_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project
#include "GbParameterWidgets.h"
#include "../../core/mixins/GbRenderable.h"
#include "../tree/GbTreeWidget.h"
#include "../../core/containers/GbSortingLayer.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RenderUniformsWidget;
class RenderLayerSelectItem;
class RenderLayerSelectWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class UniformWidget
class UniformWidget : public ParameterWidget {
    Q_OBJECT
public:
    UniformWidget(CoreEngine* core, Uniform& uniform, RenderUniformsWidget* parent = nullptr);

    virtual void update() override;

    Uniform& uniform() { return m_uniform; }

protected:

    virtual void mousePressEvent(QMouseEvent* event) override;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    void updateUniform();

    RenderUniformsWidget* m_parent = nullptr;
    Uniform& m_uniform;
    QLineEdit* m_uniformName;
    QLineEdit* m_uniformValue;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RenderUniformWidget
class RenderUniformsWidget : public ParameterWidget {
    Q_OBJECT
public:
    RenderUniformsWidget(CoreEngine* core, Shadable& renderable, QWidget* parent = nullptr);
    virtual ~RenderUniformsWidget();

    virtual void update() override;

    void setCurrentUniformWidget(UniformWidget* uniform)
    {
        m_currentUniformWidget = uniform;
    }

protected:

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    void clearUniforms();
    void repopulateUniforms();

    void addUniform();
    void removeCurrentUniform();

#ifndef QT_NO_CONTEXTMENU
    /// @brief Generates a context menu, overriding default implementation
    /// @note Context menus can be executed either asynchronously using the popup() function or 
    ///       synchronously using the exec() function
    void contextMenuEvent(QContextMenuEvent *event) override;

#endif // QT_NO_CONTEXTMENU

    Shadable& m_renderable;
    QAction* m_addUniform;
    QAction* m_removeUniform;

    UniformWidget* m_currentUniformWidget = nullptr;

    QScrollArea* m_area;
    QWidget* m_areaWidget;
    std::vector<UniformWidget*> m_uniformWidgets;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RenderSettingsWidget
class RenderSettingsWidget : public ParameterWidget {
    Q_OBJECT
public:
    RenderSettingsWidget(CoreEngine* core, 
        RenderSettings& settings, 
        QWidget* parent = nullptr);

    virtual void update() override;

protected:

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    int getDepthMode(int modeWidgetIndex) const;
    int getCulledFace(int faceWidgetIndex) const;

#ifndef QT_NO_CONTEXTMENU
    /// @brief Generates a context menu, overriding default implementation
    /// @note Context menus can be executed either asynchronously using the popup() function or 
    ///       synchronously using the exec() function
    void contextMenuEvent(QContextMenuEvent *event) override;

#endif // QT_NO_CONTEXTMENU

    RenderSettings& m_renderSettings;

    // Blend Settings
    QComboBox* m_blendMode;

    // Depth Settings
    QComboBox* m_depthMode;
    QCheckBox* m_depthTest;

    // Cull settings
    QComboBox* m_culledFace;
    QCheckBox* m_cullFace;
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class TransparencyTypeWidget
class TransparencyTypeWidget : public ParameterWidget {
    Q_OBJECT
public:
    TransparencyTypeWidget(CoreEngine* core,
        Renderable::TransparencyType& transparencyType,
        QWidget* parent = nullptr);
    ~TransparencyTypeWidget();

    virtual void update() override;

protected:

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;


    Renderable::TransparencyType& m_transparencyType;

    // Transparency type
    QComboBox* m_transparencyTypeWidget;
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RenderableWidget
template <typename T>
class RenderableWidget : public ParameterWidget {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    RenderableWidget(CoreEngine* core, T* r, QWidget* parent = nullptr) :
        ParameterWidget(core, parent),
        m_renderable(r)
    {
        initialize();
    }
    virtual ~RenderableWidget() {
        delete m_uniforms;
        delete m_renderSettings;
    }

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void update() override {
    }

    /// @}

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override {
        m_uniforms = new RenderUniformsWidget(m_engine, *m_renderable);
        m_renderSettings = new RenderSettingsWidget(m_engine, m_renderable->renderSettings());
        m_transparency = new TransparencyTypeWidget(m_engine, m_renderable->transparencyType());
    }
    virtual void initializeConnections() override {
    }
    virtual void layoutWidgets() override {
        m_mainLayout = new QVBoxLayout();
        m_mainLayout->setSpacing(0);
        m_mainLayout->addWidget(new QLabel("Uniforms:"));
        m_mainLayout->addWidget(m_uniforms);
        m_mainLayout->addWidget(new QLabel("Transparency:"));
        m_mainLayout->addWidget(m_transparency);
        m_mainLayout->addWidget(new QLabel("Render Settings:"));
        m_mainLayout->addWidget(m_renderSettings);
    }

    //virtual void updateColor();
    
    //void setLabelColor();

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    T* m_renderable;

    /// @brief Uniform combobox
    RenderUniformsWidget* m_uniforms;

    /// @brief Transparency widget
    TransparencyTypeWidget* m_transparency;

    /// @brief Render settings widget
    RenderSettingsWidget* m_renderSettings;

    /// @brief Render layer combobox
    RenderLayerSelectWidget* m_renderLayers;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif