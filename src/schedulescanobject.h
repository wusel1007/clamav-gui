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

#ifndef SCHEDULESCANOBJECT_H
#define SCHEDULESCANOBJECT_H

#include "ui_schedulescanobject.h"
#include "qroundprogressbar.h"
#include "setupfilehandler.h"
#include "highlighter.h"
#include "toolbox.h"

#include <QMessageBox>
#include <QLinearGradient>
#include <QCloseEvent>
#include <QProcess>
#include <QDialog>
#include <QTimer>
#include <QMovie>

namespace Ui {
class scheduleScanObject;
}

class scheduleScanObject : public QDialog
{
    Q_OBJECT

public:
    explicit scheduleScanObject(QWidget *parent = 0,QString name = "", QStringList parameters = QStringList());
    ~scheduleScanObject() = default;
    void closeEvent(QCloseEvent*);
    void keyPressEvent(QKeyEvent*);
    void accept();


private:
    Ui::scheduleScanObject m_ui;
    highlighter             * m_logHighLighter;
    QProcess                * m_scanProcess;
    QTimer                  * m_closeWindowTimer;
    QMovie                  * m_movie;
    QLabel                  * m_busyLabel;
    QRoundProgressBar       * m_countDown;
    setupFileHandler        * m_setupFile;
    int                       m_closeWindowCounter;
    int                       m_errorStart;
    int                       m_infectedStart;
    bool                      m_directScan;

    QString scanJob;

private slots:
    void slot_stopButtonClicked();
    void slot_closeButtonClicked();
    void slot_scanProcessHasStdOutput();
    void slot_scanProcessHasErrOutput();
    void slot_scanProcessFinished(int,QProcess::ExitStatus);
    void slot_closeWindowTimerTimeout();
    void slot_totalErrorButtonClicked();
    void slot_infectedFilesButtonClicked();

signals:
    void sendStatusReport(int,QString,QString);
    void scanProcessFinished();
    void closeRequest(QWidget*);
};

#endif // SCHEDULESCANOBJECT_H
