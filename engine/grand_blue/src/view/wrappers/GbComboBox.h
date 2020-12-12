#ifndef GB_COMBO_BOX_H
#define GB_COMBO_BOX_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt
#include <QComboBox>
#include <QPushButton>

// Project
#include "../base/GbTool.h"
#include "../style/GbFontIcon.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class EditableComboBox : public QWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{

    EditableComboBox(QWidget *parent = nullptr);

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    QComboBox* comboBox() const { return m_comboBox; }

    /// @brief Set the selected ID attribute
    void setSelectedID(const Uuid& uuid) {
        m_selectedID = uuid;
    }

    /// @brief Populate the combobox
    template<typename T>
    void populate(const std::vector<T*> objects, const char* iconName, bool block = true, bool addEmpty = true) {
        // Don't want to call signal when setting index programmatically
        if (block) {
            m_comboBox->blockSignals(true);
        }

        int count = 0;
        int index = 0;
        m_comboBox->clear();
        if (addEmpty) {
            m_comboBox->addItem("", Uuid());
            count++;
        }
        for (T* obj : objects) {
            m_comboBox->addItem(SAIcon(iconName), obj->getName(), obj->getUuid());
            if (obj->getUuid() == m_selectedID) {
                index = count;
            }
            count++;
        }

        m_comboBox->setCurrentIndex(index);

        // Unblock signals
        if (block) {
            m_comboBox->blockSignals(false);
        }
    }

    /// @}

signals:
    void currentIndexChanged(int idx);
    void editingFinished(QString text);
    void clickedAddItem();
    void removedItem(Uuid id);

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    QComboBox* m_comboBox;
    QPushButton* m_addItem;
    QPushButton* m_removeItem;

    /// @brief Cache selected UUID for when repopulating
    Uuid m_selectedID;

    /// @}


};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif





