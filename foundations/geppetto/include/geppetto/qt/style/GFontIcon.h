#pragma once

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

#include <QObject>
#include <QPainter>
#include <QIconEngine>
#include <QApplication>
#include <QtCore>
#include <QPalette>

#include "fortress/encoding/string/GEncodedString.h"
#include "fortress/json/GJson.h"

namespace rev {

class FontIcon;
class FontIconEngine;
#define FIcon(code) FontIcon::Icon(code)
#define SAIcon(stuff) FontIcon::SolidAwesomeIcon(stuff)

/// @class FontIconEngine
/// @see https://github.com/dridk/FontIcon/blob/master/FontIcon/qfonticon.cpp
class FontIconEngine : public QIconEngine {
public:
    FontIconEngine();
    ~FontIconEngine();
    virtual void paint(QPainter * painter, const QRect& rect, QIcon::Mode mode, QIcon::State state)Q_DECL_OVERRIDE;
    virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)Q_DECL_OVERRIDE;
    void setFontFamily(const QString& family) { m_fontFamily = family; }
    // define icon code using QChar or implicit using ushort ...
    void setLetter(const QChar& letter) { m_letter = letter; }
    // You can set a base color. I don't advice. Keep system color
    void setBaseColor(const QColor& baseColor) { m_baseColor = baseColor; }
    virtual QIconEngine* clone() const;

private:
    QString m_fontFamily;
    QChar m_letter;
    QColor m_baseColor;
};

/// @class FontIcon
class FontIcon : public QObject {
    Q_OBJECT

public:
    /// @brief Return metadata for font-awesome fonts
    static const json& FontAwesomeInfo() {
        return s_faInfo;
    }

    /// @brief Return a font-awesome unicode character, given the name of the icon
    /// @param fontAwesomeIcon the name of the font-awesome icon
    static GStringUtf8 FaUnicodeCharacter(const GString& fontAwesomeIcon);

    /// @brief Initialize font awesome for application
    /// @param[in] pathToJson file-path to the JSON file that contains font awesome metadata
    static void InitializeFontAwesome(const GString& pathToJson);

    /// @brief Add Font. By default, the first one is used
    static int AddFont(const QString& filename);
    static FontIcon * Instance();

    /// @brief Main methods. Return icons from code
    static QIcon GetIcon(const QChar& code, const QColor& baseColor = QColor(), const QString& family = QString());
    static QIcon SolidAwesomeIcon(const QString& iconName, const QColor& baseColor = QColor());

    /// @brief Return added fonts
    const QStringList& families() const;

protected:
    void addFamily(const QString& family);

    /// @brief Font awesome font families
    /// @note Only actually used for Qt functionality, i.e. to create QIcons
    static QString RegularFontAwesomeFamily();
    static QString BrandFontAwesomeFamily();
    static QString SolidFontAwesomeFamily();

private:
    explicit FontIcon(QObject *parent = 0);
    ~FontIcon();
    static FontIcon * m_instance;
    QStringList m_families;

    static int s_faBrands; ///< ID of Font Awesome brands in font database
    static int s_faRegular; ///< ID of Font Awesome regular in font database
    static int s_faSolid; ///< ID of Font Awesome solid in font database
    static json s_faInfo; ///< JSON document containing font-awesome icon metadata

};


} // rev
