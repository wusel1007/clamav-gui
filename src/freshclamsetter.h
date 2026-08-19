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

#ifndef FRESHCLAMSETTER_H
#define FRESHCLAMSETTER_H

#include "ui_freshclamsetter.h"
#include "setupfilehandler.h"
#include "highlighter.h"
#include "progressdialog.h"
#include "toolbox.h"

#include <QFileSystemWatcher>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFileInfo>
#include <QProcess>
#include <QWidget>
#include <QMovie>
#include <QTimer>
#include <QFile>
#include <QDir>


namespace Ui {
class freshclamsetter;
}

class freshclamsetter : public QWidget
{
    Q_OBJECT

public:
    explicit freshclamsetter(QWidget *parent = 0, setupFileHandler *setupFile = 0);
    ~freshclamsetter() = default;

private:
    Ui::freshclamsetter m_ui;
    setupFileHandler    * m_setupFile;
    setupFileHandler    * m_freshclamConf;
    QProcess            * m_updater;
    QProcess            * m_startDeamonProcess;
    //QProcess            * m_ps_process;
    //QProcess            * m_getDBUserProcess;
    QTimer              * m_startDelayTimer;
    QTimer              * m_processWatcher;
    progressDialog      * m_busyLabel;
    QFileSystemWatcher  * m_pidFileWatcher;
    QFileSystemWatcher  * m_logFileWatcher;
    QFileSystemWatcher  * m_updateLogFileWatcher;
    highlighter         * m_updateLogHighLighter;
    highlighter         * m_freshclamLogHighLighter;
    QString               m_freshclamlocationProcessOutput;
    QString               m_clamscanlocationProcessOutput;
    QString               m_pidFile;
    QString               m_logFile;
    QString               m_sudoGUI;
    bool                  m_lockFreshclamConf;
    bool                  m_startup;
    bool                  m_monochrome;
    int                   m_freshclamStartupCounter;
    void setForm(bool);
    void setUpdaterInfo();
    void checkDaemonRunning();
    QString extractPureNumber(QString value);
    QString selectColor(QString color);

private slots:
    void slot_updateNowButtonClicked();
    void slot_startStopDeamonButtonClicked();
    void slot_updaterFinished(int);
    void slot_fileSystemWatcherTriggered();
    void slot_logFileWatcherTriggered();
    void slot_updateFileWatcherTriggered();
    void slot_clearLogButtonClicked();
    void slot_clearDeamonLogButtonClicked();
    void slot_updaterHasOutput();
    void slot_startDeamonProcessFinished(int exitCode,QProcess::ExitStatus exitStatus);
    //void slot_ps_processFinished(int rc);
    void slot_disableUpdateButtons();
    void slot_startDelayTimerExpired();
    void slot_runasrootCheckBoxChanged();
    void slot_writeFreshclamSettings();
    void slot_dbPathChanged(QString dbPath);
    void slot_pidFileSelectButtonClicked();
    void slot_setFreshclamsettingsFrameState(bool state);
    void slot_autoStartDaemon();
    void slot_onUpdateExecuteButtonClicked();
    void slot_onErrorExecuteButtonClicked();
    void slot_onOutdatedExecuteButtonClicked();
    void slot_processWatcherExpired();
    void slot_add_remove_highlighter(bool state);
    void slot_initFreshclamSettings();

signals:
    void setBallonMessage(int, QString,QString);
    void disableUpdateButtons();
    void reportError();
    void updateDatabase();
    void freshclamStarted();
    void systemStatusChanged();
    void quitApplication();

};

#endif // FRESHCLAMSETTER_H
