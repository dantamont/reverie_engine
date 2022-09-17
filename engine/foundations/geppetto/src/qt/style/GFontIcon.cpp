/*
MIT License
Copyright (c) 2017 Sacha Schutz
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "geppetto/qt/style/GFontIcon.h"
#include <QDebug>
#include <QFontDatabase>

#include "fortress/encoding/string/GEncodedString.h"

#include "fonts/GFontManager.h"

namespace rev {


// FontIcon



FontIcon * FontIcon::m_instance = Q_NULLPTR;

FontIcon::FontIcon(QObject *parent)
    :QObject(parent)
{

}

FontIcon::~FontIcon()
{

}

int FontIcon::AddFont(const QString &filename)
{
    int id = QFontDatabase::addApplicationFont(filename);

    QString family = QFontDatabase::applicationFontFamilies(id).first();
    Instance()->addFamily(family);
    return id;
}

FontIcon *FontIcon::Instance()
{
    if (!m_instance)
        m_instance = new FontIcon;

    return m_instance;
}

QIcon FontIcon::GetIcon(const QChar &code, const QColor &baseColor, const QString &family)
{
    if (Instance()->families().isEmpty())
    {
        qWarning() << Q_FUNC_INFO << "No font family installed";
        return QIcon();
    }

    QString useFamily = family;
    if (useFamily.isEmpty()) {
        useFamily = Instance()->families().first();
    }


    FontIconEngine * engine = new FontIconEngine;
    engine->setFontFamily(useFamily);
    engine->setLetter(code);
    engine->setBaseColor(baseColor);
    return QIcon(engine);


}

QIcon FontIcon::SolidAwesomeIcon(const QString & iconName, const QColor & baseColor)
{
    QString fontFamily = FontIcon::SolidFontAwesomeFamily();

    GStringUtf8 unicodeChar = FontIcon::FaUnicodeCharacter(iconName.toStdString());
    Uint32_t unicode = unicodeChar.unicode();
    const QChar chr(unicode);
    if (!baseColor.isValid())
        return GetIcon(chr, QColor(255, 255, 255, 255), fontFamily);
    else
        return GetIcon(chr, baseColor, fontFamily);
}


QString FontIcon::RegularFontAwesomeFamily()
{
    QStringList families = QFontDatabase::applicationFontFamilies(s_faRegular);
    return families[0];
}

QString FontIcon::BrandFontAwesomeFamily()
{
    QStringList families = QFontDatabase::applicationFontFamilies(s_faBrands);
    return families[0];
}

QString FontIcon::SolidFontAwesomeFamily()
{
    QStringList families = QFontDatabase::applicationFontFamilies(s_faSolid);
    return families[0];
}


const QStringList &FontIcon::families() const
{
    return m_families;
}

void FontIcon::addFamily(const QString &family)
{
    m_families.append(family);
}


GStringUtf8 FontIcon::FaUnicodeCharacter(const GString& fontAwesomeIcon)
{
    assert(s_faInfo.contains(fontAwesomeIcon.c_str()) && "Error, font awesome icon not found");

    const json& unicodeJson = s_faInfo.at(fontAwesomeIcon.c_str()).at("unicode");
    return GStringUtf8(unicodeJson.get_ref<const std::string&>().c_str());
}


void FontIcon::InitializeFontAwesome(const GString& pathToJson)
{
    // Load fonts
    if (s_faBrands < 0) {
        s_faBrands = FontIcon::AddFont(":/fonts/brands-regular-400.otf");
        assert(s_faBrands >= 0 && "FontAwesome cannot be loaded!");
    }
    if (s_faRegular < 0) {
        s_faRegular = FontIcon::AddFont(":/fonts/free-regular-400.otf");
        assert(s_faRegular >= 0 && "FontAwesome cannot be loaded!");

    }
    if (s_faSolid < 0) {
        s_faSolid = FontIcon::AddFont(":/fonts/free-solid-900.otf");
        assert(s_faSolid >= 0 && "FontAwesome cannot be loaded!");
    }

    // Load font-awesome icon metadata
    GJson::FromFile(pathToJson.c_str(), s_faInfo);
}

FontIconEngine::FontIconEngine()
    :QIconEngine()
{

}

FontIconEngine::~FontIconEngine()
{
}

void FontIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(state);
    QFont font = QFont(m_fontFamily);
    int drawSize = qRound(rect.height() * 0.8);
    font.setPixelSize(drawSize);

    QColor penColor;
    if (!m_baseColor.isValid())
        penColor = QApplication::palette("QWidget").color(QPalette::Normal, QPalette::ButtonText);
    else
        penColor = m_baseColor;

    if (mode == QIcon::Disabled)
        penColor = QApplication::palette("QWidget").color(QPalette::Disabled, QPalette::ButtonText);


    if (mode == QIcon::Selected)
        penColor = QApplication::palette("QWidget").color(QPalette::Active, QPalette::ButtonText);


    painter->save();
    painter->setPen(QPen(penColor));
    painter->setFont(font);
    painter->drawText(rect, Qt::AlignCenter | Qt::AlignVCenter, m_letter);

    painter->restore();
}

QPixmap FontIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QPixmap pix(size);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    paint(&painter, QRect(QPoint(0, 0), size), mode, state);
    return pix;

}

QIconEngine *FontIconEngine::clone() const
{
    FontIconEngine * engine = new FontIconEngine;
    engine->setFontFamily(m_fontFamily);
    engine->setBaseColor(m_baseColor);
    return engine;
}


int FontIcon::s_faBrands = -1;
int FontIcon::s_faRegular = -1;
int FontIcon::s_faSolid = -1;
json FontIcon::s_faInfo = json::object();

} // rev