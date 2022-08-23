#include "geppetto/qt/widgets/general/GFileLoadWidget.h"

#include "fortress/types/GLoadable.h"
#include "fortress/json/GJson.h"
#include "fortress/containers/math/GVector.h"

#include "geppetto/qt/widgets/GWidgetManager.h"

namespace rev {

FileLoadWidget::FileLoadWidget(WidgetManager* manager,
    const QString& defaultText,
    const QString& defaultTitleText,
    const QString& selectionFilter,
    const QString& defaultDir,
    PathAccessMode loadMode,
    QWidget* parent) :
    ParameterWidget(manager, parent),
    m_defaultText(defaultText),
    m_titleText(defaultTitleText),
    m_selectionFilter(selectionFilter),
    m_loadMode(loadMode),
    m_defaultDir(defaultDir)
{
    initializeWidgets();
    initializeConnections();
    layoutWidgets();
}

void FileLoadWidget::setPath(const QString& dir)
{
    switch (m_loadMode) {
    case PathAccessMode::kMultipleFiles:
    case PathAccessMode::kSingleFile:
    {
        QFileInfo file(dir);
        if (file.exists()) {
            m_lineEdit->setText(file.absoluteFilePath());
        }
        break;
    }
    case PathAccessMode::kDirectory:
    {
        QDir direc(dir);
        if (direc.exists()) {
            m_lineEdit->setText(direc.absolutePath());
        }
        break;
    }
    default:
        assert(false && "unimplemented");
        break;
    }
}

void FileLoadWidget::initializeWidgets()
{
    m_lineEdit = new QLineEdit(m_defaultText);
    //m_lineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_button = new QPushButton();
    m_button->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    //m_button->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void FileLoadWidget::initializeConnections()
{
    connect(m_button, &QPushButton::clicked, this, [this]() {
        m_paths.clear();

        QString text;
        switch (m_loadMode) {
        case PathAccessMode::kSingleFile:
            text = QFileDialog::getOpenFileName(this, tr(m_titleText.toStdString().c_str()),
                m_defaultDir,
                tr(m_selectionFilter.toStdString().c_str()));
            m_paths.push_back(text.toStdString());
            break;
        case PathAccessMode::kMultipleFiles:
        {
            QStringList files = QFileDialog::getOpenFileNames(this, tr(m_titleText.toStdString().c_str()),
                m_defaultDir,
                tr(m_selectionFilter.toStdString().c_str()));
            for (const auto& filename : files) {
                text += filename + ";";
                m_paths.push_back(filename.toStdString());
            }
            text.chop(1);
            break;
        }
        case PathAccessMode::kDirectory:
            text = QFileDialog::getExistingDirectory(this, tr(m_titleText.toStdString().c_str()),
                m_defaultDir,
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            m_paths.push_back(text.toStdString());
            break;
        default:
            assert(false && "Error, unrecognized load mode");
        }

        m_lineEdit->setText(text);

        emit selectedFiles();
        });
}

void FileLoadWidget::layoutWidgets()
{
    m_mainLayout = new QHBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->addWidget(m_lineEdit);
    m_mainLayout->addWidget(m_button);
    m_mainLayout->setAlignment(Qt::AlignCenter);
    setLayout(m_mainLayout);
}


} // rev