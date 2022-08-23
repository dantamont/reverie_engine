/*
###############################################################################
#                                                                             #
# The MIT License                                                             #
#                                                                             #
# Copyright (C) 2017 by Juergen Skrotzky (JorgenVikingGod@gmail.com)          #
#               >> https://github.com/Jorgen-VikingGod                        #
#                                                                             #
# Sources: https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle  #
#                                                                             #
###############################################################################
*/

#ifndef GB_STYLES_H
#define GB_STYLES_H

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QProxyStyle>
#include <QStyleFactory>

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////
class DarkStyle : public QProxyStyle {
    Q_OBJECT

public:
    DarkStyle();
    explicit DarkStyle(QStyle *style);

    QStyle *baseStyle()  const {
        return styleBase();
    }

    /// @brief Style overrides, called when setting application style
    void polish(QPalette &palette) override;
    void polish(QApplication *app) override;

private:
    QStyle *styleBase(QStyle *style = Q_NULLPTR) const;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
} // rev

#endif  // DARKSTYLE_HPP
