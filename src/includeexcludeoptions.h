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

#ifndef INCLUDEEXCLUDEOPTIONS_H
#define INCLUDEEXCLUDEOPTIONS_H

#include <QWidget>
#include "setupfilehandler.h"

namespace Ui {
class includeExcludeOptions;
}

class includeExcludeOptions : public QWidget
{
    Q_OBJECT

public:
    explicit includeExcludeOptions(QWidget *parent = 0, setupFileHandler *setupFile = 0);
    ~includeExcludeOptions();

private:
    setupFileHandler    * m_setupFile;
    setupFileHandler    * m_profiles;
    Ui::includeExcludeOptions *m_ui;
    bool m_readSettingsInProgress;
    void readSettings();

private slots:
    void slot_writeSettings();
    void slot_enablePUACheckBoxClicked();
    void slot_highlightSettings();

signals:
    void updateClamdConf();
};

#endif // INCLUDEEXCLUDEOPTIONS_H
