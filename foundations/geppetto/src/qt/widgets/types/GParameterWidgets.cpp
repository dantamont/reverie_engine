#include "geppetto/qt/widgets/types/GParameterWidgets.h"

#include "fortress/types/GLoadable.h"
#include "fortress/json/GJson.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "fortress/containers/math/GVector.h"

namespace rev {

QBoxLayout* ParameterWidget::LabeledLayout(const QString& label,
    QWidget* widget,
    QBoxLayout::Direction dir) 
{
    QBoxLayout* layout = new QBoxLayout(dir);
    layout->addWidget(new QLabel(label));
    layout->addWidget(widget);
    return layout;
}

void ParameterWidget::AddLabel(const QString & label, QBoxLayout * layout)
{
    layout->addWidget(new QLabel(label));
}

void ParameterWidget::AddLabel(const QIcon & label, QBoxLayout * layout, const Vector<float, 2>& size)
{
    QLabel* l = new QLabel();
    l->setPixmap(label.pixmap(size.x(), size.y()));
    layout->addWidget(l);
}

std::vector<QWidget*> ParameterWidget::GetTopLevelWidgets(QLayout * layout)
{
    std::vector<QWidget*> widgets;
    for (int i = 0; i < layout->count(); ++i)
    {
        QWidget *widget = layout->itemAt(i)->widget();
        if (widget != nullptr)
        {
            widgets.push_back(widget);
        }
    }
    return widgets;
}

ParameterWidget::ParameterWidget(WidgetManager* manager, QWidget* parent) :
    QWidget(parent),
    m_widgetManager(manager)
{
    m_widgetManager->addParameterWidget(this);
}

ParameterWidget::~ParameterWidget()
{
    m_widgetManager->removeParameterWidget(this);
}


void ParameterWidget::initialize()
{
    this->initializeWidgets();
    this->initializeConnections();
    this->layoutWidgets();

    /// @note cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    setLayout(m_mainLayout);
}

} // rev