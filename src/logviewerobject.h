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

#ifndef LOGVIEWEROBJECT_H
#define LOGVIEWEROBJECT_H

#include <QWidget>
#include <QFile>
#include <QDir>
#include "setupfilehandler.h"
#include "partiallogobject.h"

namespace Ui {
class logViewerObject;
}

class logViewerObject : public QWidget
{
    Q_OBJECT

public:
    explicit logViewerObject(QWidget *parent = 0, setupFileHandler *setupFile = 0);
    ~logViewerObject();

private:
    setupFileHandler    *m_setupfile;
    Ui::logViewerObject *m_ui;
    QString m_logFileName;
    void loadLogFile(QString);
    void saveLog();

private slots:
    void slot_profilesChanged();
    void slot_profileSeclectionChanged();
    void slot_clearLogButtonClicked();
    void slot_clearAllButtonClicked();
    void slot_add_remove_highlighter(bool state);

signals:
    void logHighlightingChanged(bool);
};

#endif // LOGVIEWEROBJECT_H
