#pragma once

// Standard
#include <functional>

// Qt
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

// Project
#include "fortress/types/GString.h"
#include "fortress/types/GIdentifiable.h"

#include "enums/GWidgetActionTypeEnum.h"
#include "enums/GWidgetTypeEnum.h"
#include "ripple/network/messages/GMessage.h"

namespace rev {

template<typename T, size_t S> class Vector;
class WidgetManager;

/// @class ParameterWidget
class ParameterWidget : public QWidget, public IdentifiableInterface {
    Q_OBJECT
public:
    /// @name Static
    /// @{
    
    static QBoxLayout* LabeledLayout(const QString& label, QWidget* widget, QBoxLayout::Direction dir = QBoxLayout::Direction::LeftToRight);
    static void AddLabel(const QString& label, QBoxLayout* layout);
    static void AddLabel(const QIcon& label, QBoxLayout* layout, const Vector<float, 2>& size);
    static std::vector<QWidget*> GetTopLevelWidgets(QLayout* layout);

    /// @}

    /// @name Constructors and Destructors
    /// @{
    ParameterWidget(WidgetManager* manager, QWidget* parent = nullptr);
    virtual ~ParameterWidget();

    /// @}

    /// @name Public Methods
    /// @{

    bool isChild() const {
        return m_isChild;
    }

    void setIsChild(bool isChild) {
        m_isChild = isChild;
    }

    /// @brief Step-wise update of the widget
    virtual void update() {}

    /// @brief Update the parameter widget on receipt of a message
    virtual void update(class GMessage* ) {}

    /// @brief Clear the widget
    virtual void clear() {}

    /// @brief Clear a layout of widgets
    void clearLayout(QLayout *layout, bool deleteItems = true) {
        QLayoutItem *item;
        while ((item = layout->takeAt(0))) {
            if (item->layout()) {
                clearLayout(item->layout());
                if (deleteItems)
                    //delete item->layout();
                    item->layout()->deleteLater();
            }
            if (item->widget()) {
                if (deleteItems)
                    //delete item->widget();
                    item->layout()->deleteLater();
            }
            delete item;
        }
    }

    /// @}


protected:

    /// @name Protected Methods
    /// @{

    /// @brief Initialize the widget
    virtual void initialize();

    /// @brief Method to checkValidity widgets
    virtual void initializeWidgets() = 0;

    /// @brief Methods to checkValidity connections for this widget
    virtual void initializeConnections() = 0;

    /// @brief Method to ay out widgets
    virtual void layoutWidgets() = 0;

    /// @}


    /// @name Protected Members
    /// @{

    /// @brief Top-level layout for the widget
    QBoxLayout* m_mainLayout{ nullptr };

    /// @brief Pointer to the widget manager
    WidgetManager* m_widgetManager{ nullptr };

    bool m_isChild{ false }; ///< If set to true, won't update during widget manager's update loop

    /// @}

};


} // rev
