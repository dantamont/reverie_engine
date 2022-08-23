#pragma once

// Qt
#include <QtWidgets>

// External
#include "fortress/json/GJson.h"
#include "ripple/network/messages/GUpdateJsonMessage.h"

// Internal
#include "geppetto/qt/widgets/types/GParameterWidgets.h"
#include "geppetto/qt/style/GFontIcon.h"

namespace rev {

class WidgetManager;

/// @class JsonWidgetInterface
/// @brief Widget for handling JSON representation of an object
class JsonWidgetInterface : public ParameterWidget {
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    JsonWidgetInterface(WidgetManager* wm, const json& j, const json& metadata, QWidget* parent);
    virtual ~JsonWidgetInterface();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Update text from serializable JSON
    void updateText();

    /// @brief Get json representing the object
    const json& getObjectJson() const {
        return m_json["object"];
    }
    json& getObjectJson() {
        return m_json["object"];
    }

    /// @brief Set json representing the object
    void setObjectJson(const json& j) {
        m_json["object"] = j;
    }

    /// @brief Get json representing the object
    const json& getMetadata() const {
        return m_json["metadata"];
    }
    json& getMetadata() {
        return m_json["metadata"];
    }

    /// @brief Set json representing any metadata
    void setMetadata(const json& j) {
        m_json["metadata"] = j;
    }

    /// @}

signals:

    void updatedJson(const json& j);

protected:

    /// @name Private Members
    /// @{

    json m_json = { { "metadata", {}}, {"object", {}} }; ///< The JSON representing the object viewed by this widget, as well as metadata
    GUpdateJsonMessage m_message; ///< Message sent to update the item held by the JSON widget

    QCheckBox* m_toggle{ nullptr };
    QLabel* m_typeLabel{ nullptr };

    QTextEdit* m_textEdit{ nullptr };
    QPushButton* m_confirmButton{ nullptr };


    /// @}
};



/// @class JsonWidget
/// @brief Widget for handling JSON representation of an object
class JsonWidget : public JsonWidgetInterface {
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    JsonWidget(WidgetManager* wm, const json& j, const json& metadata, QWidget* parent = nullptr, bool init = false);
    virtual ~JsonWidget() = default;

    /// @}


    /// @name Public Methods
    /// @{

    /// @brief Update the parameter widget
    virtual void update(GMessage* message);

    /// @}

protected:
    /// @name Protected Events
    /// @{

    /// @brief Scroll event
    void wheelEvent(QWheelEvent* event) override;

    /// @}

    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;

    virtual void initializeConnections() override;

    virtual void layoutWidgets() override;

    /// @}
};


// End namespaces     
}
