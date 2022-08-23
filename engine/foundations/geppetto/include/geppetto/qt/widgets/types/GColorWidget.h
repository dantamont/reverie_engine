#pragma once

// Project
#include "geppetto/qt/widgets/types/GParameterWidgets.h"
#include "fortress/containers/GColor.h"
#include <QtColorWidgets/ColorDialog>

namespace rev {

/// @class ColorWidget
class ColorWidget : public ParameterWidget {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{
    ColorWidget(WidgetManager* wm, Color& color, QWidget* parent = nullptr);
    ColorWidget(WidgetManager* wm, Vector4& color, QWidget* parent = nullptr);
    virtual ~ColorWidget();

    /// @}

    /// @name Public Methods
    /// @{

    virtual void update() override;

    /// @}

signals:

    void colorChanged(const Color& color);

protected slots:

    void onColorChanged();

protected:

    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    virtual void updateColor();
    
    void setLabelColor();

    Color color() {
        if (m_color) {
            return *m_color;
        }
        else {
            return *m_colorVector;
        }
    }

    /// @}

private:

    /// @name Private Members
    /// @{

    Color* m_color = nullptr;
    Vector4* m_colorVector = nullptr;
    QPushButton* m_colorButton;
    color_widgets::ColorDialog* m_colorDialog;
    QPixmap m_labelPixmap;

    /// @}

};

} // rev
