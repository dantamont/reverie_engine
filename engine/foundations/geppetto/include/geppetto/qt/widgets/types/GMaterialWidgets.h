#pragma once

// Project
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/types/GParameterWidgets.h"
#include "geppetto/qt/widgets/tree/GTreeWidget.h"

#include "ripple/network/messages/GLoadTextureResourceMessage.h"

namespace rev {

class FileLoadWidget;

/// @class LoadTextureWidget
class LoadTextureWidget : public ParameterWidget {
public:
    /// @name Constructors and Destructors
    /// @{
    LoadTextureWidget(WidgetManager* wm, QWidget* parent = nullptr);
    virtual ~LoadTextureWidget();

    /// @}

    /// @name Public Methods
    /// @{

    virtual void update() override {
    }

    /// @}

protected:
    friend class LoadTextureCommand;

    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Protected Members
    /// @{

    GLoadTextureResourceMessage m_loadMessage;
    FileLoadWidget* m_fileLoadWidget{ nullptr };
    QDialogButtonBox* m_confirmButtons{ nullptr };

    QString m_fileName;
    Uuid m_textureHandleID = Uuid(false); // null UUID

    /// @}

};


} // rev
