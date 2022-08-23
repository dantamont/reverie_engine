#pragma once

// Project
#include "geppetto/qt/widgets/types/GParameterWidgets.h"
#include "geppetto/qt/widgets/general/GJsonWidget.h"

namespace rev {

/// @class AnimMotionWidget
class AnimMotionWidget : public ParameterWidget {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{
    AnimMotionWidget(WidgetManager* wm, json& motionJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);
    virtual ~AnimMotionWidget();

    /// @}

    /// @name Public Methods
    /// @{

    virtual void update() override;

    /// @}

protected:

    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

private:
    /// @name Private Members
    /// @{

    Uint32_t m_sceneObjectId;
    json& m_motionJson;

    QLabel* m_label{ nullptr };
    JsonWidget* m_jsonWidget{ nullptr };

    /// @}

};


} // rev
