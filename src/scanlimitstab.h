/***************************************************************************
 *   Copyright (C) 2015 by Joerg Zopes                                     *
 *   joerg.zopes@gmx.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef SCANLIMITSTAB_H
#define SCANLIMITSTAB_H

#include <QWidget>
#include "ui_scanlimitstab.h"
#include "setupfilehandler.h"

namespace Ui {
class scanLimitsTab;
}

class scanLimitsTab : public QWidget
{
    Q_OBJECT

public:
    explicit scanLimitsTab(QWidget *parent = 0, setupFileHandler * setupFile = 0);
    ~scanLimitsTab()=default;

private:
    Ui::scanLimitsTab   m_ui;
    setupFileHandler    *m_setupFile;
    setupFileHandler    *m_profiles;
    void updateLimits();
    bool update;

private slots:
    void writeLimits();

signals:
    void updateClamdConf();
};

#endif // SCANLIMITSTAB_H
