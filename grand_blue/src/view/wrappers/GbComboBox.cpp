#include "GbComboBox.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EditableComboBox::EditableComboBox(QWidget *parent) :
    QWidget(parent),
    m_comboBox(new QComboBox(this)),
    m_addItem(new QPushButton(this)),
    m_removeItem(new QPushButton(this))
{
    // Initialize widgets --------------------------------------------
    m_comboBox->setEditable(true);

    m_addItem->setIcon(SAIcon(QStringLiteral("plus")));
    m_addItem->setToolTip("Add a new item");
    m_addItem->setMaximumWidth(35);

    m_removeItem->setIcon(SAIcon(QStringLiteral("minus")));
    m_removeItem->setToolTip("Remove the selected item");
    m_removeItem->setMaximumWidth(35);

    // Initialize connections -----------------------------------------
    connect(m_comboBox,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
        m_selectedID = m_comboBox->itemData(index).toUuid();
        emit currentIndexChanged(index);
    }
    );

    connect(m_comboBox->lineEdit(),
        &QLineEdit::editingFinished, // Important that this is not textChanged
        [this]() {

        QString text = m_comboBox->lineEdit()->text();
        emit editingFinished(text);
    }
    );
    connect(m_addItem, &QAbstractButton::clicked, this,
        [this]() {
        emit clickedAddItem();
    });

    connect(m_removeItem, &QAbstractButton::clicked, this,
        [this]() {

        // Remove item from combobox
        Uuid removedID = m_comboBox->currentData().toUuid();
        m_comboBox->removeItem(m_comboBox->currentIndex());

        // Set current ID to new item's data
        m_selectedID = m_comboBox->currentData().toUuid();

        // Emit signal that item was removed
        emit removedItem(removedID);
    });

    // Layout ------------------------------------------------------------
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setSpacing(5);
    layout->addWidget(m_comboBox);
    layout->addSpacing(20);
    layout->addWidget(m_addItem);
    layout->addWidget(m_removeItem);

    // Add widgets to main layout
    setLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb