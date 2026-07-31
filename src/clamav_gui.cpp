#include "clamav_gui.h"
#include "ui_clamav_gui.h"
#include "sharedvars.cpp"

enum baloonStatus {Information, Warning, Critical};

clamav_gui::clamav_gui(QWidget* parent) : QWidget(parent)
{
    m_ui.setupUi(this);

    this->setWindowFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    firstrun = false;
    QString settingsPath = QDir::homePath() + "/.clamav-gui/settings.ini";

    if ((!QFile::exists(settingsPath)) || (!QFile::exists(QDir::homePath() + "/.clamav-gui/clamd.conf.man")) ||
        (!QFile::exists(QDir::homePath() + "/.clamav-gui/clamd.conf")) || (!QFile::exists(QDir::homePath() + "/.clamav-gui/freshclam.conf")) ||
        (!QFile::exists(QDir::homePath() + "/.clamav-gui/clamd.conf.man")))
    {
        firstrun = true;
        initDialog = new firstRunWindow(this);
        connect(initDialog,SIGNAL(rejected()),qApp,SLOT(quit()));
        initDialog->setModal(true);
        initDialog->open();

        QScreen* m_screen = QGuiApplication::primaryScreen();
        QRect m_screenGeometry = m_screen->geometry();
        initDialog->setGeometry((m_screenGeometry.width() - 650) / 2, (m_screenGeometry.height() - 410) / 2, 650,410);
    }

    if (firstrun == false)
    {
        m_error = false;
        m_guisudoapp = "pkexec";

        m_setupFile = new setupFileHandler(settingsPath, this);

        if (!firstrun) m_setupFile->setSectionValue("Setup","FirstRun",false);
        m_scanProcess = new QProcess(this);
        connect(m_scanProcess, SIGNAL(readyReadStandardError()), this, SLOT(slot_scanProcessHasErrOutput()));
        connect(m_scanProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(slot_scanProcessHasStdOutput()));
        connect(m_scanProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slot_scanProcessFinished(int, QProcess::ExitStatus)));
        if (m_setupFile->getSectionBoolValue("Settings", "ShowHideDropZone") == true)
            createDropZone();

        m_mainWindowTimer = new QTimer(this);
        connect(m_mainWindowTimer, SIGNAL(timeout()), this, SLOT(slot_mainWinTimerTimeout()));
        m_mainWindowTimer->setSingleShot(true);
        m_mainWindowTimer->start(250);

        createTrayIcon();
        m_trayIcon->setIcon(QIcon(":/icons/extra/icon32/clamav-gui.png"));
        m_trayIcon->show();
        connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
                SLOT(slot_systemTrayIconActivated(QSystemTrayIcon::ActivationReason)));

        m_ui.tabWidget->removeTab(0);

        m_scannerTab = new scanTab(this, m_setupFile);
        m_optionTab = new optionsDialog(this, m_setupFile);
        m_profileManagerTab = new ProfileManager(this, m_setupFile);
        m_schedulerTab = new scheduler(this, m_setupFile);
        m_logTab = new logViewerObject(this, m_setupFile);
        m_freshclamTab = new freshclamsetter(this, m_setupFile);
        m_clamdTab = new clamdManager(this, m_setupFile);
        m_setUpTab = new setupTab(this, m_setupFile);
        m_infoTab = new infoDialog(this);

        m_ui.tabWidget->addTab(m_scannerTab, QIcon(":/icons/icons/Clam.png"), tr("Scan"));
        m_ui.tabWidget->addTab(m_optionTab, QIcon(":/icons/icons/options.png"), tr("Options"));
        m_ui.tabWidget->addTab(m_profileManagerTab, QIcon(":/icons/icons/profilemanager.png"), tr("Profile Manager"));
        m_ui.tabWidget->addTab(m_schedulerTab, QIcon(":/icons/icons/scheduler.png"), tr("Scheduler"));
        m_ui.tabWidget->addTab(m_logTab, QIcon(":/icons/icons/includeexclude.png"), tr("Logs"));
        m_ui.tabWidget->addTab(m_freshclamTab, QIcon(":/icons/icons/freshclam.png"), tr("FreshClam"));
        m_ui.tabWidget->addTab(m_clamdTab, QIcon(":/icons/icons/onaccess.png"), tr("Clamd"));
        m_ui.tabWidget->addTab(m_setUpTab, QIcon(":/icons/icons/setup.png"), tr("Setup"));
        m_ui.tabWidget->addTab(m_infoTab, QIcon(":/icons/icons/information.png"), tr("About"));

        connect(m_scannerTab, SIGNAL(requestDropZoneVisible()), this, SLOT(slot_showDropZone()));
        connect(m_scannerTab, SIGNAL(triggerScanRequest(QStringList)), this, SLOT(slot_scanRequest(QStringList)));
        connect(m_scannerTab, SIGNAL(abortScan()), this, SLOT(slot_abortScan()));
        connect(this, SIGNAL(setScannerForm(bool)), m_scannerTab, SLOT(slot_enableForm(bool)));
        connect(m_freshclamTab, SIGNAL(quitApplication()), this, SLOT(slot_quitApplication()));

        m_ui.tabWidget->setTabShape(QTabWidget::Rounded);
        connect(m_freshclamTab, SIGNAL(setBallonMessage(int, QString, QString)), this, SLOT(slot_setTrayIconBalloonMessage(int, QString, QString)));
        connect(m_freshclamTab, SIGNAL(disableUpdateButtons()), m_freshclamTab, SLOT(slot_disableUpdateButtons()));
        connect(m_freshclamTab, SIGNAL(disableUpdateButtons()), m_scannerTab, SLOT(slot_disableScanButton()));
        connect(m_freshclamTab, SIGNAL(disableUpdateButtons()), m_schedulerTab, SLOT(slot_disableScheduler()));
        connect(m_freshclamTab, SIGNAL(reportError()), this, SLOT(slot_errorReporter()));
        connect(m_freshclamTab, SIGNAL(updateDatabase()), this, SLOT(slot_updateDatabase()));
        connect(m_freshclamTab, SIGNAL(freshclamStarted()), m_clamdTab, SLOT(slot_waitForFreshclamStarted()));
        connect(m_freshclamTab, SIGNAL(systemStatusChanged()), m_setUpTab, SLOT(slot_updateSystemInfo()));
        connect(m_clamdTab, SIGNAL(setBallonMessage(int, QString, QString)), this, SLOT(slot_setTrayIconBalloonMessage(int, QString, QString)));
        connect(m_clamdTab, SIGNAL(setActiveTab()), this, SLOT(slot_startclamd()));
        connect(m_clamdTab, SIGNAL(systemStatusChanged()), m_setUpTab, SLOT(slot_updateSystemInfo()));
        connect(m_profileManagerTab, SIGNAL(triggerProfilesChanged()), m_schedulerTab, SLOT(slot_updateProfiles()));
        connect(m_profileManagerTab, SIGNAL(triggerProfilesChanged()), m_logTab, SLOT(slot_profilesChanged()));
        connect(m_schedulerTab, SIGNAL(triggerScanJob(QString, QStringList)), this, SLOT(slot_receiveScanJob(QString, QStringList)));
        connect(m_schedulerTab, SIGNAL(logChanged()), m_logTab, SLOT(slot_profilesChanged()));
        connect(m_optionTab, SIGNAL(databasePathChanged(QString)), m_freshclamTab, SLOT(slot_dbPathChanged(QString)));
        connect(m_optionTab, SIGNAL(databasePathChanged(QString)), m_clamdTab, SLOT(slot_dbPathChanged(QString)));
        connect(m_optionTab, SIGNAL(updateDatabase()), this, SLOT(slot_updateDatabase()));
        connect(m_optionTab, SIGNAL(updateClamdConf()), m_clamdTab, SLOT(slot_updateClamdConf()));
        connect(m_optionTab, SIGNAL(systemStatusChanged()), m_setUpTab, SLOT(slot_updateSystemInfo()));
        connect(m_optionTab, SIGNAL(srtfSettingsChanged()), m_logTab, SLOT(slot_profilesChanged()));
        connect(m_setUpTab, SIGNAL(switchActiveTab(int)), this, SLOT(slot_switchActiveTab(int)));
        connect(m_setUpTab, SIGNAL(sendSystemInfo(QString)), this, SLOT(slot_receiveVersionInformation(QString)));
        connect(this, SIGNAL(scanJobFinished()), m_logTab, SLOT(slot_profilesChanged()));
        connect(this, SIGNAL(startDatabaseUpdate()), m_freshclamTab, SLOT(slot_updateNowButtonClicked()));
        connect(m_setUpTab, SIGNAL(logHighlightingChanged(bool)), m_clamdTab, SLOT(slot_add_remove_highlighter(bool)));
        connect(m_setUpTab, SIGNAL(logHighlightingChanged(bool)), m_scannerTab, SLOT(slot_add_remove_highlighter(bool)));
        connect(m_setUpTab, SIGNAL(logHighlightingChanged(bool)), m_logTab, SLOT(slot_add_remove_highlighter(bool)));
        connect(m_setUpTab, SIGNAL(logHighlightingChanged(bool)), m_freshclamTab, SLOT(slot_add_remove_highlighter(bool)));
        connect(m_setUpTab, SIGNAL(logHighlightingChanged(bool)), m_profileManagerTab, SLOT(monochromeModeChanged(bool)));
        connect(this, SIGNAL(doneit()),m_freshclamTab,SLOT(slot_initFreshclamSettings()));
        if (firstrun) connect(initDialog,SIGNAL(doneit()),m_clamdTab,SLOT(slot_initClamdSettings())); else
            connect(this,SIGNAL(doneit()),m_clamdTab,SLOT(slot_initClamdSettings()));
        if (firstrun) connect(initDialog,SIGNAL(settingChanged()),m_clamdTab,SLOT(slot_clamdSettingsChanged()));
        if (firstrun) connect(initDialog,SIGNAL(quitApplication()),this,SLOT(slot_quitApplication()));
        if (firstrun)
            connect(initDialog, SIGNAL(doneit()),m_optionTab,SLOT(slot_updateDirectories()));
        else
            connect(this,SIGNAL(doneit()),m_optionTab,SLOT(slot_updateDirectories()));

        m_ui.tabWidget->setCurrentIndex(0);

        m_logoTimer = new QTimer(this);
        m_logoTimer->setSingleShot(true);
        connect(m_logoTimer, SIGNAL(timeout()), this, SLOT(slot_logoTimerTimeout()));

        m_showLogoTimer = new QTimer(this);
        m_showLogoTimer->setSingleShot(true);
        connect(m_showLogoTimer, SIGNAL(timeout()), this, SLOT(slot_showLogoTimerTimeout()));
        m_showLogoTimer->start(250);

        m_getVersionProcess = new QProcess(this);
        connect(m_getVersionProcess,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slot_getVersionProcessFinished(int,QProcess::ExitStatus)));

        QStringList gvParams;
        gvParams << "--config-file" << QDir::homePath() + "/.clamav-gui/freshclam.conf" << "-V";

        m_getVersionProcess->start("freshclam",gvParams);

        if(!firstrun) emit doneit();
        checkAppImage();
    }
}


void clamav_gui::slot_receiveVersionInformation(QString info)
{
    m_ui.frame->setVersionLabel(info);
}

void clamav_gui::slot_getVersionProcessFinished(int, QProcess::ExitStatus)
{
    QString buffer = m_getVersionProcess->readAll();
    QStringList versionSections = buffer.split("/");
    while (versionSections.length() < 3)
        versionSections << "n/a";
    QString scannerVersion = versionSections.at(0).mid(6);
    QString value = versionSections.at(1);
    QString lastUpdate = versionSections.at(2);

    QString systemInfo = "<div style='font-size:12px;line-height:20px;'><b>Scanner: <font color='navy'>" + scannerVersion +
                 "</font><br>Database: <font color='navy'>" + value + "</font><br>";
    systemInfo += "Date: <font color='navy'>" + lastUpdate + "</font></b></div>";
    slot_receiveVersionInformation(systemInfo);
}

void clamav_gui::closeEvent(QCloseEvent* event)
{
    if (firstrun == false)
    {
        if (m_error == true)
            qApp->quit();
        else {
            if (this->isVisible() == true)
                slot_hideWindow();
            event->ignore();
        }
    }
}

void clamav_gui::changeEvent(QEvent* event)
{
    if ((event->type() == QEvent::WindowStateChange) && (isMinimized() == true))
            slot_setMainWindowState(false);
}

void clamav_gui::createTrayIcon()
{
    m_actionShowHideMainWindow = new QAction(QIcon(":/icons/icons/showhide.png"), tr("Show/Hide MainWindow"), this);
    connect(m_actionShowHideMainWindow, SIGNAL(triggered()), this, SLOT(slot_actionShowHideMainWindowTriggered()));

    m_actionShowHideDropZone = new QAction(QIcon(":/icons/icons/showhide.png"), tr("Show/Hide DropZone"), this);
    connect(m_actionShowHideDropZone, SIGNAL(triggered()), this, SLOT(slot_actionShowHideDropZoneTriggered()));

    m_actionQuit = new QAction(QIcon(":/icons/icons/application-exit.png"), tr("Quit"), this);
    connect(m_actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

    m_trayIconMenu = new QMenu(this);
    m_trayIconMenu->setStyleSheet("background:lightgray");
    m_trayIconMenu->addAction(m_actionShowHideMainWindow);
    m_trayIconMenu->addAction(m_actionShowHideDropZone);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_actionQuit);
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(m_trayIconMenu);
}

void clamav_gui::slot_actionShowHideMainWindowTriggered()
{
    if (m_setupFile->getSectionBoolValue("Settings", "ShowHideMainWindow") == true)
    {
        if (this->isActiveWindow() == true)
        {
            slot_setMainWindowState(false);
            m_setupFile->setSectionValue("Settings", "ShowHideMainWindow", false);
        }
        else {
            slot_setMainWindowState(true);
            m_setupFile->setSectionValue("Settings", "ShowHideMainWindow", true);
        }
    }
    else {
        slot_setMainWindowState(true);
        m_setupFile->setSectionValue("Settings", "ShowHideMainWindow", true);
    }
}

void clamav_gui::slot_systemTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::MiddleClick)
        slot_actionShowHideDropZoneTriggered();
    else
        if (reason == QSystemTrayIcon::Trigger)
            slot_actionShowHideMainWindowTriggered();
}

void clamav_gui::slot_setMainWindowState(bool state)
{
    if (state == true)
    {
        this->showMaximized();
        this->activateWindow();
        m_setupFile->setSectionValue("Settings", "ShowHideMainWindow", true);
    }
    else {
        if (this->isVisible() == true)
            this->hide();
        m_setupFile->setSectionValue("Settings", "ShowHideMainWindow", false);
    }
}

void clamav_gui::slot_actionShowHideDropZoneTriggered()
{
    if (m_setupFile->getSectionBoolValue("Settings", "ShowHideDropZone") == false)
    {
        m_setupFile->setSectionValue("Settings", "ShowHideDropZone", true);
        createDropZone();
    }
    else {
        m_setupFile->setSectionValue("Settings", "ShowHideDropZone", false);
        m_dropZone->close();
    }
}

void clamav_gui::slot_hideWindow()
{
    slot_setMainWindowState(false);
}

void clamav_gui::createDropZone()
{

    if (m_setupFile->getSectionBoolValue("Settings", "ShowHideMainWindow") == true)
        this->hide();

    m_dropZone = new clamav_ctrl(this);
    connect(m_dropZone, SIGNAL(scanRequest(QStringList)), this, SLOT(slot_scanRequest(QStringList)));
    m_dropZone->show();

    if (m_setupFile->getSectionBoolValue("Settings", "ShowHideMainWindow") == true)
        this->show();
}

void clamav_gui::checkAppImage()
{
    QString AppImagePath = qEnvironmentVariable("APPIMAGE");
    QString serviceMenuPath;

    if (QFileInfo::exists(QDir::homePath() + "/.local/share/kservices5/ServiceMenus/scanWithClamAV-GUI.desktop"))
        serviceMenuPath = QDir::homePath() + "/.local/share/kservices5/ServiceMenus/scanWithClamAV-GUI.desktop";

    if (serviceMenuPath.isEmpty() && QFileInfo::exists(QDir::homePath() + "/.local/share/kio/servicemenus/scanWithClamAV-GUI.desktop"))
        serviceMenuPath = QDir::homePath() + "/.local/share/kio/servicemenus/scanWithClamAV-GUI.desktop";

    if (serviceMenuPath.isEmpty() && QFileInfo::exists(QCoreApplication::applicationDirPath() + "/../share/" + "kio/servicemenues/scanWithClamAV-GUI.desktop"))
        serviceMenuPath = serviceMenuPath = QDir::homePath() + "/.local/share/kio/servicemenus/scanWithClamAV-GUI.desktop";

    if (serviceMenuPath.isEmpty() && QFileInfo::exists(QCoreApplication::applicationDirPath() + "/../share/" + "kservices5/ServiceMenus/scanWithClamAV-GUI.desktop"))
        serviceMenuPath = QDir::homePath() + "/.local/share/kservices5/ServiceMenus/scanWithClamAV-GUI.desktop";

    if (!serviceMenuPath.isEmpty())
    {
        setupFileHandler servicemenu(QDir::homePath() + "/.local/share/kio/servicemenus/scanWithClamAV-GUI.desktop");
        if (AppImagePath != "")
            servicemenu.setSectionValue("Desktop Action scan", "Exec", AppImagePath + " --scan %F");
        else
            servicemenu.setSectionValue("Desktop Action scan", "Exec", "clamav-gui --scan %F");
    }
}


/*
    TODO: refactor this method. too long...
 */
void clamav_gui::slot_scanRequest(QStringList scanObjects)
{
    QStringList selectedOptions = m_setupFile->getKeywords("SelectedOptions");
    QStringList scanLimitations = m_setupFile->getKeywords("ScanLimitations");
    QStringList directoryOptions = m_setupFile->getKeywords("Directories");
    QStringList parameters;
    QString moveDirectory = m_optionTab->getMoveDirectory();
    QString copyDirectory = m_optionTab->getCopyDirectory();
    QString checked;
    QString option;
    QString value;
    QString temp;
    bool useclamdscan = false;
    bool monochrome = m_setupFile->getSectionBoolValue("Setup", "DisableLogHighlighter");
    QString color;

    emit setScannerForm(false);
    monochrome == true ? color = "#404040;color:white" : color = "#ffff00;color:black";
    m_scannerTab->setStatusBarMessage(tr("Scanning started ......."), color);

    if (m_setupFile->getSectionValue("Clamd", "Status") == "is running")
    {
        switch (m_setupFile->getSectionIntValue("Clamd", "ClamdScanMultithreading"))
        {
            case 0:
                useclamdscan = false;
                break;
            case 1:
                useclamdscan = true;
                break;
            case 2:
                useclamdscan = false;
                break;
            case 3:
                useclamdscan = true;
                break;
            case 4:
                if (QMessageBox::question(this, tr("Use ClamdScan"), tr("Perform scanning using clamdscan instead of clamscan?"), QMessageBox::Yes,
                                          QMessageBox::No) == QMessageBox::Yes)
                    useclamdscan = true;
                else
                    useclamdscan = false;
        }
    }
    else {
        useclamdscan = false;
    }

    switch (m_scannerTab->getVirusFoundComboBoxValue())
    {
        case 0:
            break;
        case 1:
            parameters << "--remove=yes";
            temp = temp + " --remove=yes";
            break;
        case 2:
            if (moveDirectory != "")
            {
                parameters << "--move=" + moveDirectory;
                temp = temp + " --move=" + moveDirectory;
            }
            break;
        case 3:
            if (moveDirectory != "")
            {
                parameters << "--copy=" + copyDirectory;
                temp = temp + " --copy=" + copyDirectory;
            }
            break;
    }

    if (useclamdscan == true)
    {
        temp = "clamdscan --config-file=" + m_setupFile->getSectionValue("Clamd","ClamdConfPath") + " --multiscan --fdpass";
        parameters << "--config-file=" + m_setupFile->getSectionValue("Clamd","ClamdConfPath") << "--multiscan" << "--fdpass";

        value = m_setupFile->getSectionValue("Directories", "ScanReportToFile");
        checked = value.left(value.indexOf("|"));
        value = value.mid(value.indexOf("|") + 1);
        if ((checked == "checked") && (value != ""))
        {
            parameters << "--log=" + value;
            temp = temp + " --log=" + value;
            QFile file(value);
            if (file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
            {
                QTextStream stream(&file);
                stream << "\n<Scanning startet> " << QDateTime::currentDateTime().toString("yyyy/M/d - hh:mm");
                file.close();
            }
        }
        QStringList  scandParameters;
        scandParameters << "--infected" << "--stdout" << "--no-summary";
        foreach (QString param, scandParameters)
        {
            if (m_setupFile->keywordExists("SelectedOptions", param) == true)
            {
                parameters << param;
                temp = temp + " " + param;
            }
        }
    }
    else {
        if (m_scannerTab->recursivChecked() == true)
            parameters << "-r";

        for (int i = 0; i < selectedOptions.count(); i++)
        {
            parameters << selectedOptions.at(i).left(selectedOptions.indexOf("|")).replace("<equal>", "=");
        }

        // Directory Options
        for (int i = 0; i < directoryOptions.count(); i++)
        {
            option = directoryOptions.at(i);
            value = m_setupFile->getSectionValue("Directories", option);
            checked = value.left(value.indexOf("|"));
            value = value.mid(value.indexOf("|") + 1);

            if ((checked == "checked") && (value != ""))
            {
                for (int idx = 0; idx < directoryOptionKeywords.size(); idx++)
                {
                    if (directoryOptionKeywords.at(idx) == option)
                    {
                        if (directoryOptionKeywords.at(idx) == "ScanReportToFile")
                        {
                            if (value != "")
                            {
                                parameters << "--log=" + value;
                                QFile file(value);
                                if (file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
                                {
                                    QTextStream stream(&file);
                                    stream << "\n<Scanning startet> " << QDateTime::currentDateTime().toString("yyyy/M/d - hh:mm");
                                    file.close();
                                }
                            }
                        }
                        else
                            parameters << directoryOptionSwitches.at(idx) + "=" + value;
                    }
                }
            }
        }

        // Scan Limitations
        for (int i = 0; i < scanLimitations.count(); i++)
        {
            option = scanLimitations.at(i);
            value = m_setupFile->getSectionValue("ScanLimitations", option);
            checked = value.left(value.indexOf("|"));
            value = value.mid(value.indexOf("|") + 1);
            if (checked == "checked")
            {
                for (int i = 0; i < scanLimitKeywords.length(); i++)
                {
                    if (option == scanLimitKeywords.at(i))
                        parameters << scanLimitSwitches.at(i) + "=" + value;
                }
            }
        }

        // REGEXP and Include Exclude Options
        for (int idx = 0; idx < inclExclKeywords.size(); idx++)
        {
            if (idx < 4)
            {
                value = m_setupFile->getSectionValue("REGEXP_and_IncludeExclude",inclExclKeywords.at(idx));
                checked = value.left(value.indexOf("|"));
                value = value.mid(value.indexOf("|") + 1);
                if (checked == "checked") parameters << inclExclSwitches.at(idx) + "=" + value;
            }
            else {
                if (m_setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","EnablePUAOptions") == true)
                {
                    if ((m_setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude",inclExclKeywords.at(idx)) == true) &&
                        (inclExclKeywords.at(idx) != "EnablePUAOptions"))
                        parameters << inclExclSwitches.at(idx);
                }
            }
        }

        temp = (useclamdscan == true)?"clamdscan":"clamscan";

        QString path = temp;
        path = m_setupFile->getSectionValue("Directories", "LoadSupportedDBFiles");
        path = path.mid(path.indexOf("|") + 1);

        if (path.indexOf("not checked") != -1)
            temp = temp + "-d " + path;

        for (int i = 0; i < parameters.count(); i++)
        {
            temp = temp + " " + parameters.at(i);
        }
    }

    foreach (QString parameter, scanObjects)
    {
        parameters << parameter;
        temp = temp + " " + parameter;
    }

    slot_setMainWindowState(true);

    if (m_setupFile->getSectionBoolValue("Settings", "ShowHideDropZone") == true)
        m_dropZone->hide();

    m_ui.tabWidget->setCurrentIndex(0);

    m_scannerTab->clearLogMessage();
    m_scannerTab->setStatusMessage(temp + char(13));

    (useclamdscan == true)?m_scanProcess->start("clamdscan", parameters):m_scanProcess->start("clamscan", parameters);
}

void clamav_gui::slot_mainWinTimerTimeout()
{
    if (m_setupFile->getSectionBoolValue("Settings", "ShowHideMainWindow") == true)
        slot_setMainWindowState(true);
    else
        slot_setMainWindowState(false);
}

void clamav_gui::slot_scanProcessHasStdOutput()
{
    QString output = m_scanProcess->readAllStandardOutput();
    m_scannerTab->setStatusMessage(output);
}

void clamav_gui::slot_scanProcessHasErrOutput()
{
    QString output = m_scanProcess->readAllStandardError();
    m_scannerTab->setStatusMessage(output);
}

void clamav_gui::slot_scanProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(status);
    QStringList m_parameters;
    bool monochrome = m_setupFile->getSectionBoolValue("Setup", "DisableLogHighlighter");
    QString m_color;

    if (status == QProcess::CrashExit)
    {
        monochrome == true ? m_color = "#404040;color:white" : m_color = "#ff0000;color:white";
        m_scannerTab->setStatusBarMessage(tr("Scan-Process aborted ......"), m_color);
        m_trayIcon->showMessage(tr("Scan-Status"), tr("Scan Process aborted ....."), QSystemTrayIcon::Warning, 5000);
    }
    else {
        if (exitCode == 0)
        {
            monochrome == true ? m_color = "#404040;color:white" : m_color = "#00ff00;color:white";
            m_scannerTab->setStatusBarMessage(tr("Scan-Process finished ...... no Virus found!"), m_color);
            m_trayIcon->showMessage(tr("Scan-Status"), tr("Scan Process finished ..... no virus found!"), QSystemTrayIcon::Information, 5000);
        }
        else
            if (exitCode == 1)
            {
                monochrome == true ? m_color = "#404040;color:white" : m_color = "#ff0000;color:white";
                m_scannerTab->setStatusBarMessage(tr("Scan-Process finished ...... Virus found!"), m_color);
                m_trayIcon->showMessage(tr("Scan-Status"), tr("Scan Process finished ..... a virus was found!"), QSystemTrayIcon::Critical, 5000);
            }
            else {
                monochrome == true ? m_color = "#404040;color:white" : m_color = "#ff0000;color:white";
                m_scannerTab->setStatusBarMessage(tr("Scan-Process finished ...... an error occured!"), m_color);
                m_trayIcon->showMessage(tr("Scan-Status"), tr("Scan Process finished ..... an error occurred!"), QSystemTrayIcon::Warning, 5000);
            }
        }

    emit setScannerForm(true);

    if (m_setupFile->getSectionBoolValue("Settings", "ShowHideDropZone") == true)
    {
        createDropZone();
    }

    emit scanJobFinished();
}

void clamav_gui::slot_abortScan()
{
    bool monochrome = m_setupFile->getSectionBoolValue("Setup", "DisableLogHighlighter");
    QString color;

    monochrome == true ? color = "#404040;color:white" : color = "#ff0000;color:white";
    m_scannerTab->setStatusMessage(tr("Scan-Process aborted!"));
    m_scannerTab->setStatusBarMessage(tr("Scan-Process aborted!"), color);

    if (m_scanProcess->state() == QProcess::Running)
        m_scanProcess->kill();
}

void clamav_gui::slot_showDropZone()
{
    if (m_setupFile->getSectionBoolValue("Settings", "ShowHideDropZone") == false)
    {
        m_setupFile->setSectionValue("Settings", "ShowHideDropZone", true);
        createDropZone();
    }
}

void clamav_gui::slot_receiveScanJob(QString name, QStringList m_parameters)
{
    // we create a scanObject but when did they are destroyed?
    scheduleScanObject* scanObject = new scheduleScanObject(this, name, m_parameters);

    scanObject->setWindowTitle(name);
    scanObject->setWindowIcon(QIcon(":/icons/icons/media.png"));
    connect(scanObject, SIGNAL(sendStatusReport(int, QString, QString)), this, SLOT(slot_setTrayIconBalloonMessage(int, QString, QString)));
    connect(scanObject, SIGNAL(scanProcessFinished()), m_logTab, SLOT(slot_profilesChanged()));
    connect(scanObject, SIGNAL(closeRequest(QWidget*)), this, SLOT(slot_closeRequest(QWidget*)));
    scanObject->setModal(false);
    scanObject->show();
}

void clamav_gui::slot_setTrayIconBalloonMessage(int status, QString title, QString message)
{

    //TODO: use enumerate - done!
    switch (status)
    {
        case Information:
            m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 5000);
            break;
        case Warning:
            m_trayIcon->showMessage(title, message, QSystemTrayIcon::Warning);
            break;
        case Critical:
            m_trayIcon->showMessage(title, message, QSystemTrayIcon::Critical);
            break;
    }
}

void clamav_gui::slot_closeRequest(QWidget* dialog)
{
    delete dialog;
}

void clamav_gui::slot_logoTimerTimeout()
{
    m_startLogoLabel->hide();
    if (m_setupFile->keywordExists("Setup", "WindowState"))
    {
        if (m_setupFile->getSectionValue("Setup", "WindowState") == "minimized")
            this->close();
        if (m_setupFile->getSectionValue("Setup", "WindowState") == "maximized")
        {
            this->showMaximized();
            m_setupFile->setSectionValue("Settings", "ShowHideMainWindow", true);
        }
    }
    else
        this->showMaximized();
}

void clamav_gui::slot_showLogoTimerTimeout()
{
    QScreen* m_screen = QGuiApplication::primaryScreen();
    QRect m_screenGeometry = m_screen->geometry();

    m_startLogoLabel = new QLabel(this);
    m_startLogoLabel->setStyleSheet("background:transparent");
    m_startLogoLabel->setAttribute(Qt::WA_TranslucentBackground);
    m_startLogoLabel->setWindowFlags(Qt::FramelessWindowHint);
    m_startLogoLabel->setPixmap(QPixmap(":/icons/icons/startlogo.png"));
    m_startLogoLabel->setGeometry((m_screenGeometry.width() - 400) / 2, (m_screenGeometry.height() - 300) / 2, 400, 300);
    m_startLogoLabel->show();
    m_startLogoLabel->raise();

    m_logoTimer->start(2500);
}

void clamav_gui::slot_errorReporter()
{
    m_error = true;
}

void clamav_gui::slot_updateDatabase()
{
    m_ui.tabWidget->setCurrentIndex(5);
    emit startDatabaseUpdate();
}

void clamav_gui::slot_startclamd()
{
    m_ui.tabWidget->setCurrentIndex(6);
}

void clamav_gui::slot_switchActiveTab(int index)
{
    m_ui.tabWidget->setCurrentIndex(index);
}

void clamav_gui::slot_quitApplication()
{
    qApp->quit();
}
