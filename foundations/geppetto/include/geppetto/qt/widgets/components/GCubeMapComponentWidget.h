#pragma once

// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidget.h"
#include "geppetto/qt/widgets/types/GRenderableWidget.h"
#include "geppetto/qt/widgets/types/GParameterWidgets.h"

#include "fortress/containers/GColor.h"

#include "ripple/network/messages/GModifyCubemapColorMessage.h"
#include "ripple/network/messages/GModifyCubemapNameMessage.h"
#include "ripple/network/messages/GModifyCubemapTextureMessage.h"
#include "ripple/network/messages/GModifyDefaultCubemapMessage.h"

namespace rev {

class ColorWidget;
class FileLoadWidget;

/// @class CubeMapComponentWidget
/// @brief Widget representing a cubemap component
class CubeMapComponentWidget : public SceneObjectComponentWidget {
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    CubeMapComponentWidget(WidgetManager* wm, const json& componentJson, Int32_t sceneObjectId, QWidget *parent = 0);
    ~CubeMapComponentWidget();

    /// @}

    /// @name Public methods
    /// @{

    virtual void update() override;

    /// @}

public slots:
signals:
private slots:
private:
    /// @name Private Methods
    /// @{

    void requestChangeCubemapColor(const Color& color);
    void requestChangeCubemapName(const std::string& name);
    void requestChangeCubemapTexture(const std::string& path);
    void requestSetDefaultCubemap(bool isDefault);

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Private Members
    /// @{

    Color m_cachedDiffuseColor;

    GModifyCubemapColorMessage m_colorMessage;
    GModifyCubemapNameMessage m_nameMessage;
    GModifyCubemapTextureMessage m_textureMessage;
    GModifyDefaultCubemapMessage m_defaultCubemapMessage;

    QLineEdit* m_nameEdit{ nullptr };
    QCheckBox* m_isDefaultCubeMap{ nullptr };
    FileLoadWidget* m_textureLoad{ nullptr };
    ColorWidget* m_colorWidget{ nullptr };

    /// @}
};

// End namespaces
}
