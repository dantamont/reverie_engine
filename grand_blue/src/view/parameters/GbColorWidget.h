#ifndef GB_COLOR_WIDGET_H 
#define GB_COLOR_WIDGET_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project
#include "GbParameterWidgets.h"
#include "../../core/containers/GbColor.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ColorWidget
class ColorWidget : public ParameterWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ColorWidget(CoreEngine* core, Color& color, QWidget* parent = nullptr);
    ColorWidget(CoreEngine* core, Vector4g& color, QWidget* parent = nullptr);
    virtual ~ColorWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void update() override;

    /// @}

protected slots:

    void onColorChanged();

protected:

    //-----------------------------------------------------------------------------------------------------------------
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
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    Color* m_color = nullptr;
    Vector4g* m_colorVector = nullptr;
    QPushButton* m_colorButton;
    QColorDialog* m_colorDialog;
    QPixmap m_labelPixmap;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif