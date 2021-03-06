#ifndef GB_VECTOR_WIDGET_H 
#define GB_VECTOR_WIDGET_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project
#include "GParameterWidgets.h"
#include "../../core/geometry/GVector.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class VectorWidget
/// @brief Note that this does not employ the Q_Object macro, which is incompatible with templates
template<typename T, size_t N>
class VectorWidget : public ParameterWidget {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    VectorWidget(CoreEngine* core, Vector<T, N>& vec,
        QWidget* parent = nullptr,
        double minVal = -1e20,
        double maxVal = 1e20,
        double numDecimals = 15,
        const QStringList& labels = {"x:", "y:", "z:", "w:"},
        size_t numToShow = N) :
        ParameterWidget(core, parent),
        m_vector(vec),
        m_labelStrings(labels),
        m_minAllowed(minVal),
        m_maxAllowed(maxVal),
        m_numDecimals(numDecimals),
        m_numToShow(numToShow)
    {
        initialize();
    }
    virtual ~VectorWidget() {
    }

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Reinitialize with a different vector
    void reinitialize(Vector<T, N>& vec) {
        m_vector = vec;
        update();
    }

    /// @brief Update the parameter widget
    virtual void update() override {
        for (size_t i = 0; i < m_numToShow; i++) {
            QLineEdit* lineEdit = m_lineEdits[i];
            if (!lineEdit->hasFocus()) {
                lineEdit->setText(QString::number(m_vector[i]));
            }
        }
    }

    /// @}


protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Method to checkValidity widgets
    virtual void initializeWidgets() override {
        if (m_labels.size()) throw("Need to delete labels");

        for (size_t i = 0; i < m_numToShow; i++) {
            m_labels.push_back(new QLabel(m_labelStrings[i % N]));
        }

        for (size_t i = 0; i < m_numToShow; i++) {
            m_lineEdits.push_back(new QLineEdit(QString::number(m_vector[i])));
            m_lineEdits.back()->setMaximumWidth(75);
            m_lineEdits.back()->setValidator(
                new QDoubleValidator(m_minAllowed, m_maxAllowed, m_numDecimals));
        }
    }

    /// @brief Methods to checkValidity connections for this widget
    virtual void initializeConnections() override {
        for (const auto& lineEdit : m_lineEdits) {
            connect(lineEdit, &QLineEdit::editingFinished,
                [this]() { 
                updateVector();
            }
            );
        }
    }

    /// @brief Method to ay out widgets
    virtual void layoutWidgets() override {
        m_mainLayout = new QHBoxLayout();
        m_mainLayout->setSpacing(0);

        for (size_t i = 0; i < m_numToShow; i++) {
            QLabel* label = m_labels[i];
            QLineEdit* lineEdit = m_lineEdits[i];
            m_mainLayout->addWidget(label);
            m_mainLayout->addWidget(lineEdit);
            if (i != m_numToShow - 1)
                m_mainLayout->addSpacing(15);
        }
    }

    /// @brief Update the value of the underlying vector from the widget values
    virtual void updateVector() {
        for (size_t i = 0; i < m_numToShow; i++) {
            QLineEdit* lineEdit = m_lineEdits[i];
            m_vector[i] = lineEdit->text().toDouble();
        }
    }

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    Vector<T, N>& m_vector;

    std::vector<QLabel*> m_labels;
    QStringList m_labelStrings;
    std::vector<QLineEdit*> m_lineEdits;

    double m_minAllowed;
    double m_maxAllowed;
    double m_numDecimals;
    size_t m_numToShow;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // rev

#endif