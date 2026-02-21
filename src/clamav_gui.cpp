#include "clamav_gui.h"
#include "ui_clamav_gui.h"

clamav_gui::clamav_gui(QWidget* parent) : QWidget(parent)
{
    m_ui.setupUi(this);
    this->setWindowFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    QString settingsPath = QDir::homePath() + "/.clamav-gui/settings.ini";
    bool createDefaultSettings = false;
    m_error = false;
    m_guisudoapp = "pkexec";
    //*****************************************************************************
    //creating service Menu
    //*****************************************************************************
    QDir dir;
    QString serviceMenuPath;
    if (QFileInfo::exists(QDir::homePath() + "/.local/share/kservices5/ServiceMenus")) {
        serviceMenuPath = QDir::homePath() + "/.local/share/kservices5/ServiceMenus";
    }
    if (serviceMenuPath.isEmpty() && QFileInfo::exists(QDir::homePath() + "/.local/share/kio/servicemenus")) {
        serviceMenuPath = QDir::homePath() + "/.local/share/kio/servicemenus";
    }
    if (serviceMenuPath.isEmpty()) {
        serviceMenuPath = serviceMenuPath = QDir::homePath() + "/.local/share/kservices5/ServiceMenus";
    }
    if (!QFileInfo::exists(serviceMenuPath + "/scanWithClamAV-GUI.desktop")) {
        if (!QFileInfo::exists(serviceMenuPath)) {
            QDir dir(serviceMenuPath);
            dir.mkdir(serviceMenuPath);
        }
        setupFileHandler* serviceFile = new setupFileHandler(serviceMenuPath + "/scanWithClamAV-GUI.desktop", this);
        serviceFile->setSectionValue("Desktop Entry", "Type", "Service");
        serviceFile->setSectionValue("Desktop Entry", "ServiceTypes", "KonqPopupMenu/Plugin");
        serviceFile->setSectionValue("Desktop Entry", "MimeType", "all/all;");
        serviceFile->setSectionValue("Desktop Entry", "Actions", "scan;");
        serviceFile->setSectionValue("Desktop Entry", "Icon", "clamav-gui");
        serviceFile->setSectionValue("Desktop Entry", "X-KDE-Priority", "TopLevel");
        serviceFile->setSectionValue("Desktop Entry", "X-KDE-StartupNotify", "false");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu", "Scan with ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[de]", "Scannen mit ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[da_DK]", "Scannen med ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[es_ES]", "Analizar con ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[us]", "Scan with ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[gb]", "Scan with ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[pt]", "Investigar com ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[br]", "Investigar com ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[pt_BR]", "Investigar com ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[fr]", "Scanner avec ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[it]", "Scansione con ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[uk]", "Сканування за допомогою ClamAV-GUI");

        serviceFile->setSectionValue("Desktop Action scan", "Name", "scan");
        serviceFile->setSectionValue("Desktop Action scan", "Name[de]", "Scannen mit ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[es_ES]", "Analizar con ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[us]", "Scan with ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[gb]", "Scan with ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[pt]", "Investigar com ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[br]", "Investigar com ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[fr]", "Scanner avec ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[it]", "Scansione con ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[uk]", "Сканування за допомогою ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Icon", "clamav-gui");
        serviceFile->setSectionValue("Desktop Action scan", "Exec", "clamav-gui --scan %F");
        delete serviceFile;
    }
    //*****************************************************************************

    //*****************************************************************************
    // Creating default direcotry structur in HOME
    //*****************************************************************************
    if (!QFileInfo::exists(QDir::homePath() + "/.clamav-gui")) {
        dir.mkdir(QDir::homePath() + "/.clamav-gui");
        createDefaultSettings = true;
    }
    if (!QFileInfo::exists(QDir::homePath() + "/.clamav-gui/quarantine")) {
        dir.mkdir(QDir::homePath() + "/.clamav-gui/quarantine");
    }
    if (!QFileInfo::exists(QDir::homePath() + "/.clamav-gui/signatures")) {
        dir.mkdir(QDir::homePath() + "/.clamav-gui/signatures");
    }
    if (!QFileInfo::exists(QDir::homePath() + "/.clamav-gui/profiles")) {
        dir.mkdir(QDir::homePath() + "/.clamav-gui/profiles");
    }
    if (!QFileInfo::exists(QDir::homePath() + "/.clamav-gui/logs")) {
        dir.mkdir(QDir::homePath() + "/.clamav-gui/logs");
    }
    QFile::setPermissions(QDir::homePath() + "/.clamav-gui/signatures",
                          QFileDevice::ReadOwner | QFileDevice::ReadUser | QFileDevice::ReadGroup | QFileDevice::ReadOther | QFileDevice::WriteOwner |
                              QFileDevice::WriteUser | QFileDevice::WriteGroup | QFileDevice::WriteOther | QFileDevice::ExeOwner |
                              QFileDevice::ExeUser | QFileDevice::ExeGroup | QFileDevice::ExeOther);
    //*****************************************************************************

    //_____________________________________________________________________________________________________________________________________
    // For UALinux
    // If the settings.ini in the home folder of the user is not present a predefined version is been copied into the folder.
    //_____________________________________________________________________________________________________________________________________
    if (!QFileInfo::exists(QDir::homePath() + "/.clamav-gui/settings.ini")) {
        dir.mkdir(QDir::homePath() + "/.clamav-gui");
        if (QFile::exists("/etc/clamav-gui/settings.ini")) {
            QFile::copy("/etc/clamav-gui/settings.ini", QDir::homePath() + "/.clamav-gui/settings.ini");
        }
    }
    //______________________________________________________________________________________________________________________________________

    m_setupFile = new setupFileHandler(settingsPath, this);
    if (createDefaultSettings) {
        if ((QFileInfo::exists("/var/lib/clamav")) && (QFile::exists("/var/lib/clamav/freshclam.dat"))) {
            m_setupFile->setSectionValue("Directories", "LoadSupportedDBFiles", "checked|/var/lib/clamav");
        }
        else {
            m_setupFile->setSectionValue("Directories", "LoadSupportedDBFiles", "checked|" + QDir::homePath() + "/.clamav-gui/signatures");
        }
        if (m_setupFile->keywordExists("Directories", "TmpFile") == false)
            m_setupFile->setSectionValue("Directories", "TmpFile", "checked|/tmp");
        if (m_setupFile->keywordExists("Directories", "ScanReportToFile") == false)
            m_setupFile->setSectionValue("Directories", "ScanReportToFile", "checked|" + QDir::homePath() + "/.clamav-gui/logs/report-scan.log");
        if (m_setupFile->keywordExists("Directories", "MoveInfectedFiles") == false)
            m_setupFile->setSectionValue("Directories", "MoveInfectedFiles", "not checked|" + QDir::homePath() + "/.clamav-gui/quarantine");
        if (m_setupFile->keywordExists("Directories", "CopyInfectedFiles") == false)
            m_setupFile->setSectionValue("Directories", "CopyInfectedFiles", "not checked|" + QDir::homePath() + "/.clamav-gui/quarantine");
        if (m_setupFile->keywordExists("Setup", "language") == false)
            m_setupFile->setSectionValue("Setup", "language", 2);
    }

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
    connect(m_scannerTab, SIGNAL(requestDropZoneVisible()), this, SLOT(slot_showDropZone()));
    connect(m_scannerTab, SIGNAL(triggerScanRequest(QStringList)), this, SLOT(slot_scanRequest(QStringList)));
    connect(m_scannerTab, SIGNAL(abortScan()), this, SLOT(slot_abortScan()));
    connect(this, SIGNAL(setScannerForm(bool)), m_scannerTab, SLOT(slot_enableForm(bool)));
    m_ui.tabWidget->addTab(m_scannerTab, QIcon(":/icons/icons/Clam.png"), tr("Scan"));

    m_optionTab = new optionsDialog(this, m_setupFile);
    m_ui.tabWidget->addTab(m_optionTab, QIcon(":/icons/icons/options.png"), tr("Options"));

    m_profileManagerTab = new ProfileManager(this, m_setupFile);
    m_ui.tabWidget->addTab(m_profileManagerTab, QIcon(":/icons/icons/profilemanager.png"), tr("Profile Manager"));

    m_schedulerTab = new scheduler(this, m_setupFile);
    m_ui.tabWidget->addTab(m_schedulerTab, QIcon(":/icons/icons/scheduler.png"), tr("Scheduler"));

    m_logTab = new logViewerObject(this, m_setupFile);
    m_ui.tabWidget->addTab(m_logTab, QIcon(":/icons/icons/includeexclude.png"), tr("Logs"));

    m_freshclamTab = new freshclamsetter(this, m_setupFile);
    connect(m_freshclamTab, SIGNAL(quitApplication()), this, SLOT(slot_quitApplication()));
    m_ui.tabWidget->addTab(m_freshclamTab, QIcon(":/icons/icons/freshclam.png"), tr("FreshClam"));

    m_clamdTab = new clamdManager(this, m_setupFile);
    m_ui.tabWidget->addTab(m_clamdTab, QIcon(":/icons/icons/onaccess.png"), tr("Clamd"));

    m_setUpTab = new setupTab(this, m_setupFile);
    m_ui.tabWidget->addTab(m_setUpTab, QIcon(":/icons/icons/setup.png"), tr("Setup"));

    m_infoTab = new infoDialog(this);
    m_ui.tabWidget->addTab(m_infoTab, QIcon(":/icons/icons/information.png"), tr("About"));

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
    connect(m_optionTab, SIGNAL(updateDatabase()), this, SLOT(slot_updateDatabase()));
    connect(m_optionTab, SIGNAL(updateClamdConf()), m_clamdTab, SLOT(slot_updateClamdConf()));
    connect(m_optionTab, SIGNAL(systemStatusChanged()), m_setUpTab, SLOT(slot_updateSystemInfo()));
    connect(m_setUpTab, SIGNAL(switchActiveTab(int)), this, SLOT(slot_switchActiveTab(int)));
    connect(m_setUpTab, SIGNAL(sendSystemInfo(QString)), this, SLOT(slot_receiveVersionInformation(QString)));
    connect(this, SIGNAL(scanJobFinished()), m_logTab, SLOT(slot_profilesChanged()));
    connect(this, SIGNAL(startDatabaseUpdate()), m_freshclamTab, SLOT(slot_updateNowButtonClicked()));
    connect(m_setUpTab, SIGNAL(logHighlightingChanged(bool)), m_clamdTab, SLOT(slot_add_remove_highlighter(bool)));
    connect(m_setUpTab, SIGNAL(logHighlightingChanged(bool)), m_scannerTab, SLOT(slot_add_remove_highlighter(bool)));
    connect(m_setUpTab, SIGNAL(logHighlightingChanged(bool)), m_logTab, SLOT(slot_add_remove_highlighter(bool)));
    connect(m_setUpTab, SIGNAL(logHighlightingChanged(bool)), m_freshclamTab, SLOT(slot_add_remove_highlighter(bool)));
    connect(m_setUpTab, SIGNAL(logHighlightingChanged(bool)), m_profileManagerTab, SLOT(monochromeModeChanged(bool)));

    m_ui.tabWidget->setCurrentIndex(0);

    m_logoTimer = new QTimer(this);
    m_logoTimer->setSingleShot(true);
    connect(m_logoTimer, SIGNAL(timeout()), this, SLOT(slot_logoTimerTimeout()));

    m_showLogoTimer = new QTimer(this);
    m_showLogoTimer->setSingleShot(true);
    connect(m_showLogoTimer, SIGNAL(timeout()), this, SLOT(slot_showLogoTimerTimeout()));
    m_showLogoTimer->start(250);

    m_sudoGUIProcess = new QProcess(this);
    connect(m_sudoGUIProcess, SIGNAL(finished(int)), this, SLOT(slot_sudoGUIProcessFinished()));
    QStringList m_parameters;
    m_parameters << "pkexec";
    m_sudoGUIProcess->start("whereis", m_parameters);
}

void clamav_gui::slot_receiveVersionInformation(QString info)
{
    m_ui.frame->setVersionLabel(info);
}

void clamav_gui::closeEvent(QCloseEvent* event)
{
    if (m_error == true) {
        qApp->quit();
    }
    else {
        if (this->isVisible() == true)
            slot_hideWindow();
        event->ignore();
    }
}

void clamav_gui::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized() == true) {
            slot_setMainWindowState(false);
        }
    }
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
    if (m_setupFile->getSectionBoolValue("Settings", "ShowHideMainWindow") == true) {
        slot_setMainWindowState(false);
        m_setupFile->setSectionValue("Settings", "ShowHideMainWindow", false);
    }
    else {
        slot_setMainWindowState(true);
        m_setupFile->setSectionValue("Settings", "ShowHideMainWindow", true);
    }
}

void clamav_gui::slot_systemTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::MiddleClick) {
        slot_actionShowHideDropZoneTriggered();
    }
    else {
        if (reason == QSystemTrayIcon::Trigger) {
            slot_actionShowHideMainWindowTriggered();
        }
    }
}

void clamav_gui::slot_setMainWindowState(bool state)
{
    if (state == true) {
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
    if (m_setupFile->getSectionBoolValue("Settings", "ShowHideDropZone") == false) {
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

/*
    TODO: refactor this method. too long...
 */
void clamav_gui::slot_scanRequest(QStringList scanObjects)
{
    QStringList selectedOptions = m_setupFile->getKeywords("SelectedOptions");
    QStringList scanLimitations = m_setupFile->getKeywords("ScanLimitations");
    QStringList directoryOptions = m_setupFile->getKeywords("Directories");
    QStringList parameters;
    QStringList keywords;
    QStringList switches;
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
    monochrome == true ? color = "#404040;m_color:white" : color = "#ffff00";
    m_scannerTab->setStatusBarMessage(tr("Scanning started ......."), color);

    if (m_setupFile->getSectionValue("Clamd", "Status") == "is running") {
        switch (m_setupFile->getSectionIntValue("Clamd", "ClamdScanMultithreading")) {
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
                                          QMessageBox::No) == QMessageBox::Yes) {
                    useclamdscan = true;
                }
                else {
                    useclamdscan = false;
                }
        }
    }
    else {
        useclamdscan = false;
    }

    if (useclamdscan == true) {
        temp = "clamdscan --config-file " + QDir::homePath() + "/.clamav-gui/clamd.conf --multiscan --fdpass";
        parameters << "--config-file" << QDir::homePath() + "/.clamav-gui/clamd.conf" << "--multiscan" << "--fdpass";

        for (int i = 0; i < scanObjects.count(); i++) {
            parameters << scanObjects.at(i);
            temp = temp + " " + scanObjects.at(i);
        }

        slot_setMainWindowState(true);

        if (m_setupFile->getSectionBoolValue("Settings", "ShowHideDropZone") == true) {
            m_dropZone->close();
        }

        m_ui.tabWidget->setCurrentIndex(0);

        m_scannerTab->clearLogMessage();
        m_scannerTab->setStatusMessage(temp + char(13));

        m_scanProcess->start("clamdscan", parameters);
    }
    else {

        if (m_scannerTab->recursivChecked() == true) {
            parameters << "-r";
        }

        switch (m_scannerTab->getVirusFoundComboBoxValue()) {
            case 0:
                break;
            case 1:
                parameters << "--remove=yes";
                break;
            case 2:
                if (moveDirectory != "")
                    parameters << "--move=" + moveDirectory;
                break;
            case 3:
                if (moveDirectory != "")
                    parameters << "--copy=" + copyDirectory;
                break;
        }

        for (int i = 0; i < selectedOptions.count(); i++) {
            parameters << selectedOptions.at(i).left(selectedOptions.indexOf("|")).replace("<equal>", "=");
        }

        keywords << "TmpFile" << "MoveInfectedFiles" << "CopyInfectedFiles" << "SCanFileFromFiles" << "FollowDirectorySymLinks"
                 << "FollowFileSymLinks";
        switches << "--tempdir=" << "--move=" << "--copy=" << "--file-list=" << "--follow-dir-symlinks=" << "--follow-file-symlinks=";
        // Directory Options
        for (int i = 0; i < directoryOptions.count(); i++) {
            option = directoryOptions.at(i);
            value = m_setupFile->getSectionValue("Directories", option);
            checked = value.left(value.indexOf("|"));
            value = value.mid(value.indexOf("|") + 1);

            if ((checked == "checked") && (value != "")) {
                if (option == "LoadSupportedDBFiles") {
                    parameters << "--database=" + value;
                }
                if (option == "ScanReportToFile") {
                    parameters << "--log=" + value;
                    if (value != "") {
                        QFile file(value);
                        if (file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text)) {
                            QTextStream stream(&file);
                            stream << "\n<Scanning startet> " << QDateTime::currentDateTime().toString("yyyy/M/d - hh:mm");
                            file.close();
                        }
                    }
                }
                if (keywords.indexOf(option) != -1)
                    parameters << switches.at(keywords.indexOf(option)) + value;
            }
        }

        // Scan Limitations
        QStringList SLKeywords;
        QStringList SLSwitches;
        SLKeywords << "Files larger than this will be skipped and assumed clean" << "The maximum amount of data to scan for each container file";
        SLKeywords << "The maximum number of files to scan for each container file" << "Maximum archive recursion level for container file";
        SLKeywords << "Maximum directory recursion level" << "Maximum size file to check for embedded PE"
                   << "Maximum size of HTML file to normalize";
        SLKeywords << "Maximum size of normalized HTML file to scan" << "Maximum size of script file to normalize"
                   << "Maximum size zip to type reanalyze";
        SLKeywords << "Maximum number of partitions in disk image to be scanned" << "Maximum number of icons in PE file to be scanned";
        SLKeywords << "Number of seconds to wait for waiting a response back from the stats server" << "Bytecode timeout in milliseconds";
        SLKeywords << "Collect and print execution statistics" << "Structured SSN Format" << "Structured SSN Count" << "Structured CC Count"
                   << "Structured CC Mode";
        SLKeywords << "Max Scan-Time" << "Max recursion to HWP3 parsing function" << "Max calls to PCRE match function"
                   << "Max recursion calls to the PCRE match function";
        SLKeywords << "Max PCRE file size" << "Database outdated if older than x days";
        SLSwitches << "--max-filesize=" << "--max-scansize=" << "--max-files=" << "--max-recursion=" << "--max-dir-recursion=";
        SLSwitches << "--max-embeddedpe=" << "--max-htmlnormalize=" << "--max-htmlnotags=" << "--max-scriptnormalize=" << "--max-ziptypercg=";
        SLSwitches << "--max-partitions=" << "--max-iconspe=" << "--stats-timeout=" << "--bytecode-timeout=" << "--statistics=";
        SLSwitches << "--structured-ssn-format=" << "--structured-ssn-count=" << "--structured-cc-count=" << "--structured-cc-mode="
                   << "--max-scantime=";
        SLSwitches << "--max-rechwp3=" << "--pcre-match-limit=" << "--pcre-recmatch-limit=" << "--pcre-max-filesize="
                   << "--fail-if-cvd-older-than=";
        for (int i = 0; i < scanLimitations.count(); i++) {
            option = scanLimitations.at(i);
            value = m_setupFile->getSectionValue("ScanLimitations", option);
            checked = value.left(value.indexOf("|"));
            value = value.mid(value.indexOf("|") + 1);
            if (checked == "checked") {
                for (int i = 0; i < SLKeywords.length(); i++) {
                    if (option == SLKeywords.at(i)) {
                        parameters << SLSwitches.at(i) + value;
                    }
                }
            }
        }

        // REGEXP and Include Exclude Options
        value = m_setupFile->getSectionValue("REGEXP_and_IncludeExclude", "DontScanFileNamesMatchingRegExp");
        checked = value.left(value.indexOf("|"));
        value = value.mid(value.indexOf("|") + 1);
        if (checked == "checked")
            parameters << "--exclude=" + value;

        value = m_setupFile->getSectionValue("REGEXP_and_IncludeExclude", "DontScanDirectoriesMatchingRegExp");
        checked = value.left(value.indexOf("|"));
        value = value.mid(value.indexOf("|") + 1);
        if (checked == "checked")
            parameters << "--exclude-dir=" + value;

        value = m_setupFile->getSectionValue("REGEXP_and_IncludeExclude", "OnlyScanFileNamesMatchingRegExp");
        checked = value.left(value.indexOf("|"));
        value = value.mid(value.indexOf("|") + 1);
        if (checked == "checked")
            parameters << "--include=" + value;

        value = m_setupFile->getSectionValue("REGEXP_and_IncludeExclude", "OnlyScanDirectoriesMatchingRegExp");
        checked = value.left(value.indexOf("|"));
        value = value.mid(value.indexOf("|") + 1);
        if (checked == "checked")
            parameters << "--include-dir=" + value;

        if (m_setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude", "EnablePUAOptions") == true) {
            QStringList m_keywords;
            QStringList m_switches;
            m_keywords << "LoadPUAPacked" << "LoadPUAPWTool" << "LoadPUANetTool" << "LoadPUAP2P" << "LoadPUAIRC" << "LoadPUARAT"
                       << "LoadPUANetToolSpy";
            m_keywords << "LoadPUAServer" << "LoadPUAScript" << "LoadPUAAndr" << "LoadPUAJava" << "LoadPUAOsx" << "LoadPUATool" << "LoadPUAUnix"
                       << "LoadPUAWin";
            m_switches << "--include-pua=Packed" << "--include-pua=PWTool" << "--include-pua=NetTool" << "--include-pua=P2P" << "--include-pua=IRC"
                       << "--include-pua=RAT";
            m_switches << "--include-pua=NetToolSpy" << "--include-pua=Server" << "--include-pua=Script" << "--include-pua=Andr"
                       << "--include-pua=Java";
            m_switches << "--include-pua=Osx" << "--include-pua=Tool" << "--include-pua=Unix" << "--include-pua=Win";
            for (int i = 0; i < m_keywords.length(); i++) {
                if (m_setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude", m_keywords.at(i)) == true)
                    parameters << m_switches.at(i);
            }
        }

        temp = "clamscan ";
        QString path = temp;
        path = m_setupFile->getSectionValue("Directories", "LoadSupportedDBFiles");
        path = path.mid(path.indexOf("|") + 1);
        if (path.indexOf("not checked") != -1)
            temp = temp + "-d " + path;
        for (int i = 0; i < parameters.count(); i++) {
            temp = temp + " " + parameters.at(i);
        }

        // m_parameters << "-d" << m_path;
        for (int i = 0; i < scanObjects.count(); i++) {
            parameters << scanObjects.at(i);
            temp = temp + " " + scanObjects.at(i);
        }

        slot_setMainWindowState(true);

        if (m_setupFile->getSectionBoolValue("Settings", "ShowHideDropZone") == true) {
            m_dropZone->close();
        }

        m_ui.tabWidget->setCurrentIndex(0);

        m_scannerTab->clearLogMessage();
        m_scannerTab->setStatusMessage(temp + char(13));

        m_scanProcess->start("clamscan", parameters);
    }
}

void clamav_gui::slot_mainWinTimerTimeout()
{
    if (m_setupFile->getSectionBoolValue("Settings", "ShowHideMainWindow") == true) {
        slot_setMainWindowState(true);
    }
    else {
        slot_setMainWindowState(false);
    }
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

    if (status == QProcess::CrashExit) {
        monochrome == true ? m_color = "#404040;m_color:white" : m_color = "#ff0000";
        m_scannerTab->setStatusBarMessage(tr("Scan-Process aborted ......"), m_color);
        m_trayIcon->showMessage(tr("Scan-Status"), tr("Scan Process aborted ....."), QSystemTrayIcon::Warning, 5000);
    }
    else {
        if (exitCode == 0) {
            monochrome == true ? m_color = "#404040;m_color:white" : m_color = "#00ff00";
            m_scannerTab->setStatusBarMessage(tr("Scan-Process finished ...... no Virus found!"), m_color);
            m_trayIcon->showMessage(tr("Scan-Status"), tr("Scan Process finished ..... no virus found!"), QSystemTrayIcon::Information, 5000);
        }
        else if (exitCode == 1) {
            monochrome == true ? m_color = "#404040;m_color:white" : m_color = "#ff0000";
            m_scannerTab->setStatusBarMessage(tr("Scan-Process finished ...... Virus found!"), m_color);
            m_trayIcon->showMessage(tr("Scan-Status"), tr("Scan Process finished ..... a virus was found!"), QSystemTrayIcon::Critical, 5000);
        }
        else {
            monochrome == true ? m_color = "#404040;m_color:white" : m_color = "#ff0000";
            m_scannerTab->setStatusBarMessage(tr("Scan-Process finished ...... an error occured!"), m_color);
            m_trayIcon->showMessage(tr("Scan-Status"), tr("Scan Process finished ..... an error occurred!"), QSystemTrayIcon::Warning, 5000);
        }
    }

    emit setScannerForm(true);

    if (m_setupFile->getSectionBoolValue("Settings", "ShowHideDropZone") == true) {
        createDropZone();
    }

    emit scanJobFinished();
}

void clamav_gui::slot_abortScan()
{
    bool monochrome = m_setupFile->getSectionBoolValue("Setup", "DisableLogHighlighter");
    QString color;

    monochrome == true ? color = "#404040;m_color:white" : color = "#ff0000";
    m_scannerTab->setStatusMessage(tr("Scan-Process aborted!"));
    m_scannerTab->setStatusBarMessage(tr("Scan-Process aborted!"), color);

    if (m_scanProcess->state() == QProcess::Running)
        m_scanProcess->kill();
}

void clamav_gui::slot_showDropZone()
{
    if (m_setupFile->getSectionBoolValue("Settings", "ShowHideDropZone") == false) {
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
    //TODO: use enumerate
    switch (status) {
        // 0 = Information, 1 = Warning, 2 = Critical
        case 0:
            m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 5000);
            break;
        case 1:
            m_trayIcon->showMessage(title, message, QSystemTrayIcon::Warning);
            break;
        case 2:
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
    if (m_setupFile->keywordExists("Setup", "WindowState")) {
        if (m_setupFile->getSectionValue("Setup", "WindowState") == "minimized")
            this->close();
        if (m_setupFile->getSectionValue("Setup", "WindowState") == "maximized") {
            this->showMaximized();
        }
    }
    else {
        this->showMaximized();
    }
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

void clamav_gui::slot_sudoGUIProcessFinished()
{
    QStringList m_parameters;

    QString m_sudoGUIOutput = m_sudoGUIProcess->readAll();

    if (m_guisudoapp == "pkexec") {
        if (m_sudoGUIOutput != "pkexec:\n") {
            QStringList m_values = m_sudoGUIOutput.split(" ");
            if (m_values.size() > 1) {
                if (m_values.length() > 0)
                    m_setupFile->setSectionValue("Settings", "SudoGUI", m_values[1]);
            }
            else {
                m_guisudoapp = "kdesu";
                m_parameters << "kdesu";
                m_sudoGUIProcess->start("whereis", m_parameters);
            }
        }
    }

    if (m_guisudoapp == "kdesu") {
        if (m_sudoGUIOutput != "kdesu:\n") {
            QStringList m_values = m_sudoGUIOutput.split(" ");
            if (m_values.size() > 1) {
                if (m_values.length() > 0)
                    m_setupFile->setSectionValue("Settings", "SudoGUI", m_values[1]);
            }
            else {
                QMessageBox::warning(this, tr("WARNING"),
                                     tr("Neither \"pkexec\" nor \"kdesu\" is installed. Please install at least one of this to apps!"));
            }
        }
    }
}

void clamav_gui::slot_switchActiveTab(int index)
{
    m_ui.tabWidget->setCurrentIndex(index);
}

void clamav_gui::slot_quitApplication()
{
    qApp->quit();
}
