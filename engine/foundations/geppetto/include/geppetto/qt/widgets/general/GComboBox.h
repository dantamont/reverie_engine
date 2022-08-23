#pragma once

// Qt
#include <QComboBox>
#include <QPushButton>

// Project
#include "fortress/encoding/uuid/GUuid.h"

#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/layer/types/GQtConverter.h"

namespace rev {

/// @class EditableComboBox
class EditableComboBox : public QWidget {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{

    EditableComboBox(QWidget *parent = nullptr);

    /// @}

    /// @name Public Methods
    /// @{

    QComboBox* comboBox() const { return m_comboBox; }

    /// @brief Set the selected ID attribute
    void setSelectedID(const Uuid& uuid) {
        m_selectedID = uuid;
    }

    /// @brief Populate the combobox
    void populate(const json& objectArray, const char* iconName, bool block = true, bool addEmpty = true);

    /// @}

signals:
    void currentIndexChanged(int idx);
    void editingFinished(QString text);
    void clickedAddItem();
    void removedItem(Uuid id);

protected:
    /// @name Protected Members
    /// @{

    QComboBox* m_comboBox;
    QPushButton* m_addItem;
    QPushButton* m_removeItem;
    Uuid m_selectedID; ///< Cache selected UUID for when repopulating

    /// @}


};

} // rev
