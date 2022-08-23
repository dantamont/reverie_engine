#pragma once

// Public
#include "geppetto/qt/widgets/types/GParameterWidgets.h"

namespace rev {

template<typename T, size_t S> class Vector;

enum class PathAccessMode {
    kSingleFile,
    kMultipleFiles,
    kDirectory
};

/// @class FileLoadWidget
/// @brief Widget with interface for loading a file
class FileLoadWidget : public ParameterWidget {
    Q_OBJECT
public:

    /// @name Constructors and Destructors
    /// @{
    FileLoadWidget(WidgetManager* manager,
        const QString& defaultText = QString(), 
        const QString& defaultTitleText = "Open File",
        const QString& selectionFilter = QString(),
        const QString& defaultPath = ".",
        PathAccessMode loadMode = PathAccessMode::kSingleFile,
        QWidget* parent = nullptr);

    /// @}

    /// @name Public Methods
    /// @{

    const std::vector<GString>& fileNames() const { return m_paths; }

    /// @brief Line edit widget
    QLineEdit* lineEdit() const { return m_lineEdit; }


    /// @brief Get the path string stored in the line edit widget
    QString getPath() const {
        return m_lineEdit->text();
    }

    /// @brief Set the path in the line edit
    void setPath(const QString& dir);

    /// @}

signals:

    /// @brief Emitted when files are selected
    void selectedFiles();

protected:

    /// @name Protected Methods
    /// @{

    /// @brief Method to checkValidity widgets
    virtual void initializeWidgets() override;

    /// @brief Methods to checkValidity connections for this widget
    virtual void initializeConnections() override;

    /// @brief Method to ay out widgets
    virtual void layoutWidgets() override;

    /// @}


    /// @name Protected Members
    /// @{

    /// @brief Cache files selected by the widget
    std::vector<GString> m_paths;

    PathAccessMode m_loadMode;

    /// @brief Line Edit for file/directory path
    QLineEdit* m_lineEdit;

    QString m_titleText;
    QString m_selectionFilter;
    QString m_defaultDir;

    /// @brief Button for selecting file/directory
    QPushButton* m_button;

    /// @brief Default text for the widget
    QString m_defaultText;

    /// @}

};


} // rev
