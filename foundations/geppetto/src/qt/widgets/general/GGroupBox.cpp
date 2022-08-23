#include "geppetto/qt/widgets/general/GGroupBox.h"

#include <QStylePainter>
#include <QStyleOptionGroupBox>

namespace rev {


GroupBox::GroupBox(const QString &title, QWidget *parent) :
    QGroupBox(title, parent),
    m_disableFrame(false)
{
}

GroupBox::GroupBox(QWidget * parent, bool disableFrame) :
    QGroupBox(parent),
    m_disableFrame(disableFrame)
{
}

void GroupBox::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QStylePainter paint(this);
    QStyleOptionGroupBox option;
    initStyleOption(&option);

    // This should disable frame painting.
    if (m_disableFrame) {
        option.features = QStyleOptionFrame::None;
    }

    paint.drawComplexControl(QStyle::CC_GroupBox, option);
}


} // rev