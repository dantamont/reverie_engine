#ifndef GB_PARAMETER_WIDGETS_H 
#define GB_PARAMETER_WIDGETS_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes
#include <functional>

// Qt
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>

// Project
#include "../base/GbTool.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class CoreEngine;
class Serializable;

namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ParameterWidget
class ParameterWidget : public QWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ParameterWidget(CoreEngine* core, QWidget* parent = nullptr);

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @}


protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Initialize the widget
    virtual void initialize();

    /// @brief Method to checkValidity widgets
    virtual void initializeWidgets() {}

    /// @brief Methods to checkValidity connections for this widget
    virtual void initializeConnections() {}

    /// @brief Method to ay out widgets
    virtual void layoutWidgets() {}

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Top-level layout for the widget
    QBoxLayout* m_mainLayout;

    /// @brief Pointer to core engine
    CoreEngine* m_engine;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class FileLoadWidget
/// @brief Wiget with interface for loading a file
class FileLoadWidget : public ParameterWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    FileLoadWidget(CoreEngine* core, 
        const QString& defaultText = QString(), 
        const QString& defaultTitleText = "Open File",
        const QString& selectionFilter = QString(),
        QWidget* parent = nullptr);

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Line edit widget
    QLineEdit* lineEdit() const { return m_lineEdit; }

    /// @}


protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Method to checkValidity widgets
    virtual void initializeWidgets() override;

    /// @brief Methods to checkValidity connections for this widget
    virtual void initializeConnections() override;

    /// @brief Method to ay out widgets
    virtual void layoutWidgets() override;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Line Edit for file/directory path
    QLineEdit* m_lineEdit;

    QString m_titleText;
    QString m_selectionFilter;

    /// @brief Button for selecting file/directory
    QPushButton* m_button;

    /// @brief Pointer to core engine
    CoreEngine* m_engine;

    /// @brief Default text for the widget
    QString m_defaultText;

    /// @}

};




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class JsonWidget
class JsonWidget : public ParameterWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    JsonWidget(CoreEngine* core, Serializable* serializable, QWidget *parent = 0);
    ~JsonWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Update text from serializable JSON
    void updateText(bool reloadJson = false);

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief What to do prior to reloading serializable
    virtual void preLoad() {}

    /// @brief What do do after reloading serializable
    virtual void postLoad() {}


    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    QCheckBox* m_toggle;
    QLabel* m_typeLabel;

    QTextEdit* m_textEdit;
    QPushButton* m_confirmButton;

    /// @brief Serializable object
    Serializable* m_serializable;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif