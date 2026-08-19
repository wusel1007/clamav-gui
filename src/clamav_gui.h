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

#ifndef CLAMAV_GUI_H
#define CLAMAV_GUI_H

#include "setupfilehandler.h"
#include "optionsdialog.h"
#include "clamav_ctrl.h"
#include "ui_clamav_gui.h"
#include "freshclamsetter.h"
#include "scantab.h"
#include "setuptab.h"
#include "scheduler.h"
#include "infodialog.h"
#include "schedulescanobject.h"
#include "logviewerobject.h"
#include "profilemanager.h"
#include "clamdmanager.h"
#include "firstrunwindow.h"
#include "toolbox.h"

#include <QSystemTrayIcon>
#include <QTextStream>
#include <QStringList>
#include <QPixmap>
#include <QProcess>
#include <QWidget>
#include <QAction>
#include <QTimer>
#include <QDebug>
#include <QScreen>
#include <QTimer>
#include <QMenu>
#include <QList>
#include <QDir>


namespace Ui {
class clamav_gui;
}

class clamav_gui : public QWidget
{
    Q_OBJECT

public:
    explicit clamav_gui(QWidget *parent = 0);
    void closeEvent(QCloseEvent *);
    void changeEvent(QEvent *event);

    ~clamav_gui() = default;

private:
    Ui::clamav_gui          m_ui;
    QSystemTrayIcon         * m_trayIcon;
    QMenu                   * m_trayIconMenu;
    QAction                 * m_actionQuit;
    QAction                 * m_actionShowHideDropZone;
    QAction                 * m_actionShowHideMainWindow;
    setupFileHandler        * m_setupFile;
    QProcess                * m_scanProcess;
    QProcess                * m_getVersionProcess;
    QString                   m_guisudoapp;
    scanTab                 * m_scannerTab;
    setupTab                * m_setUpTab;
    QTimer                  * m_mainWindowTimer;
    QTimer                  * m_logoTimer;
    QTimer                  * m_showLogoTimer;
    optionsDialog           * m_optionTab;
    scanLimitsTab           * m_scanLimitTab;
    freshclamsetter         * m_freshclamTab;
    clamdManager            * m_clamdTab;
    ProfileManager          * m_profileManagerTab;
    scheduler               * m_schedulerTab;
    infoDialog              * m_infoTab;
    logViewerObject         * m_logTab;
    clamav_ctrl             * m_dropZone;
    QLabel                  * m_startLogoLabel;
    firstRunWindow          * initDialog;
    bool                     m_error;
    bool                     firstrun;
    void createTrayIcon();
    void createDropZone();
    void checkAppImage();

private slots:
    void slot_setMainWindowState(bool);
    void slot_actionShowHideMainWindowTriggered();
    void slot_systemTrayIconActivated(QSystemTrayIcon::ActivationReason);
    void slot_actionShowHideDropZoneTriggered();
    void slot_hideWindow();
    void slot_scanRequest(QStringList);
    void slot_mainWinTimerTimeout();
    void slot_scanProcessHasStdOutput();
    void slot_scanProcessHasErrOutput();
    void slot_scanProcessFinished(int,QProcess::ExitStatus);
    void slot_abortScan();
    void slot_showDropZone();
    void slot_receiveScanJob(QString, QStringList);
    void slot_setTrayIconBalloonMessage(int,QString, QString);
    void slot_closeRequest(QWidget*);
    void slot_logoTimerTimeout();
    void slot_showLogoTimerTimeout();
    void slot_errorReporter();
    void slot_updateDatabase();
    void slot_startclamd();
    void slot_switchActiveTab(int index);
    void slot_quitApplication();
    void slot_receiveVersionInformation(QString info);
    void slot_getVersionProcessFinished(int, QProcess::ExitStatus);

signals:
    void showHideDropZoneTriggered();
    void setScannerForm(bool);
    void scanJobFinished();
    void startDatabaseUpdate();
    void doneit();
};

#endif // CLAMAV_GUI_H
