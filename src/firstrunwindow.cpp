#include "firstrunwindow.h"
#include "ui_firstrunwindow.h"

firstRunWindow::firstRunWindow(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::firstRunWindow)
{
    m_ui->setupUi(this);
    this->setWindowTitle("ClamAV-GUI Basic Settings");

    m_processInit = true;

    createServiceMenu();
    createBaseDirStructure();

    m_setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");
    m_setupFile->setSectionValue("Setup","FirstRun",true);
    createInitialSettings();
    createClamdConfFile();
    createFreshclamConfFile();
    m_delayTimer = new QTimer(this);
    m_delayTimer->setSingleShot(true);
    connect(m_delayTimer,SIGNAL(timeout()),this,SLOT(slot_findRequiredApplications()));
    m_delayTimer->start(500);
    findTranslation();
    slot_sddComboBoxChanged();

    m_processInit = false;
}

firstRunWindow::~firstRunWindow()
{
    delete m_setupFile;
    delete m_ui;
}

void firstRunWindow::slot_findRequiredApplications()
{
    m_initCommands << "whereis" << "whereis" << "whereis" << "whereis" << "whereis" << "whereis" << "whereis" << "whoami" << "groups" << "man";
    m_initParameters  << "clamd" << "freshclam"<< "clamonacc" << "clamscan" << "clamdscan" << "pkexec" << "kdesu" << "" << "" << "clamd.conf";
    m_initIndex = 0;

    m_initProcess = new QProcess(this);
    connect(m_initProcess,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slot_initProcessFinished()));

    m_processParameters.clear();
    m_processParameters << m_initParameters.at(m_initIndex);
    m_initProcess->start(m_initCommands.at(m_initIndex),m_processParameters);
}

void firstRunWindow::slot_monochromeModeChanged()
{
    m_setupFile->setSectionValue("Setup", "DisableLogHighlighter", m_ui->monochromeModeCheckBox->isChecked());
}

void firstRunWindow::slot_startupModeChanged()
{
    if (m_ui->startupModeComboBox->currentIndex() == 0)
        m_setupFile->setSectionValue("Setup", "WindowState", "maximized");
    if (m_ui->startupModeComboBox->currentIndex() == 1)
        m_setupFile->setSectionValue("Setup", "WindowState", "minimized");
}

void firstRunWindow::slot_clamdscanChanged()
{
    m_setupFile->setSectionValue("Clamd","ClamdScanMultithreading",m_ui->clamdscanComboBox->currentIndex());
}

void firstRunWindow::slot_gsettingsProcessFinished(int rc, QProcess::ExitStatus)
{
    QString output = m_gsettingsProcess->readAll();
    QStringList gnomecommanderParams;
    if (rc == 0)
    {
        if (output.indexOf("[]") != -1)
        {
            gnomecommanderParams << "set"  << "org.gnome.gnome-commander.preferences.general" << "favorite-apps" << "[('scan with ClamAV-GUI', '/usr/bin/clamav-gui --scan %F', '/usr/share/icons/hicolor/48x48/apps/clamav-gui.png', '', uint32 2, false, true, false)]";
            QProcess::execute("gsettings",gnomecommanderParams);
        }
        else {
            if (output.indexOf("clamav-gui") == -1)
            {
                output = output.mid(0,output.length() - 2) + ", ('scan with ClamAV-GUI', '/usr/bin/clamav-gui --scan %F', '/usr/share/icons/hicolor/48x48/apps/clamav-gui.png', '', uint32 2, false, true, false)]";
            }
            gnomecommanderParams << "set" << "org.gnome.gnome-commander.preferences.general" << "favorite-apps"  << output;
            QProcess::execute("gsettings",gnomecommanderParams);
        }
    }
}

void firstRunWindow::findTranslation()
{
    int index = -1;
    QString m_country = "";
    QString translation_path = QCoreApplication::applicationDirPath() + "/../share/clamav-gui/";
    //QString translation_path = "/usr/share/clamav-gui/"; //for testing ...
    QDir directory(translation_path);
    QStringList m_filelist = directory.entryList(QDir::Files);
    foreach(QString m_file, m_filelist)
    {
        if (m_file.indexOf(".qm") != -1 && m_file.contains("gui"))
        {
            QString m_lang = m_file.mid(11,5);
            QLocale locale(m_lang);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
            m_country = locale.territoryToString(locale.territory());
#else
            m_country = locale.countryToString(locale.country());
#endif

            m_ui->applicationLanguageComboBox->addItem(QIcon(translation_path + "languageicons/" + m_lang + ".png"),"[" + m_lang + "] " + m_country);
        }
    }

    QString lang = QLocale::system().name();
    index = m_ui->applicationLanguageComboBox->findText("[" + lang + "]", Qt::MatchContains);
    if (index == -1)
        index = m_ui->applicationLanguageComboBox->findText("[en_GB]", Qt::MatchContains);
    if (index != -1)
        m_ui->applicationLanguageComboBox->setCurrentIndex(index);
 }

void firstRunWindow::slot_initProcessFinished()
{
    QString rc = m_initProcess->readAll().trimmed();
    QStringList elements = rc.split(" ");

    if (m_initIndex < m_initCommands.size())
    {
        switch (m_initIndex)
        {
            case 0:
                if (elements.count() >= 2)
                {
                    m_ui->clamdSourceLabel->setText(elements.at(1));
                    m_ui->clamdStatusLabel->setPixmap(QPixmap(":/icons/icons/create.png"));
                    m_setupFile->setSectionValue("Clamd","ClamdLocation",elements.at(1));
                    m_setupFile->setSectionValue("Clamd","StartClamdOnStartup",false);
                }
                else {
                    QMessageBox::warning(this, tr("ERROR"), tr("Clamad is missing. Please install!"), QMessageBox::Ok);
                    emit quitApplication();
                }
                break;
            case 1:
                if (elements.count() >= 2)
                {
                    m_ui->freshclamSourceLabel->setText(elements.at(1));
                    m_ui->freshclamStatusLabel->setPixmap(QPixmap(":/icons/icons/create.png"));
                    m_setupFile->setSectionValue("FreshclamSettings", "FreshclamLocation", elements.at(1));
                }
                else {
                    QMessageBox::warning(this, tr("ERROR"), tr("Freshclam is missing. Please install!"), QMessageBox::Ok);
                    emit quitApplication();
                }
                break;
            case 2:
                if (elements.count() >= 2)
                {
                    m_ui->clamonaccSourceLabel->setText(elements.at(1));
                    m_ui->clamonaccStatusLabel->setPixmap(QPixmap(":/icons/icons/create.png"));
                    m_setupFile->setSectionValue("Clamd","ClamonaccLocation",elements.at(1));
                }
                else {
                    QMessageBox::warning(this, tr("ERROR"), tr("Clamonacc is missing. Please install!"), QMessageBox::Ok);
                    emit quitApplication();
                }
                break;
            case 3:
                if (elements.count() >=2)
                {
                    m_ui->clamscanSourceLabel->setText(elements.at(1));
                    m_ui->clamscanStatusLabel->setPixmap(QPixmap(":/icons/icons/create.png"));
                }
                else {
                    QMessageBox::warning(this, tr("ERROR"), tr("Clamav is missing. Please install!"), QMessageBox::Ok);
                    emit quitApplication();
                }
                break;
            case 4:
                if (elements.count() >= 2)
                {
                    m_ui->clamdscanSourceLabel->setText(elements.at(1));
                    m_ui->clamdscanStatusLabel->setPixmap(QPixmap(":/icons/icons/create.png"));
                }
                break;
            case 5:
                if (elements.count() >= 2)
                {
                    m_ui->sudoGUISourceLabel->setText(elements.at(1));
                    m_ui->sudoGUIStatusLabel->setPixmap(QPixmap(":/icons/icons/create.png"));
                    m_setupFile->setSectionValue("RequiredApplications",m_initParameters.at(m_initIndex),elements.at(1));
                    m_setupFile->setSectionValue("Settings","SudoGUI",elements.at(1));
                    m_initIndex = 6;
                }
                break;
            case 6:
                if (elements.count() >= 2)
                {
                    m_ui->sudoGUISourceLabel->setText(elements.at(1));
                    m_ui->sudoGUIStatusLabel->setPixmap(QPixmap(":/icons/icons/create.png"));
                    m_setupFile->setSectionValue("RequiredApplications",m_initParameters.at(m_initIndex),elements.at(1));
                    m_setupFile->setSectionValue("Settings","SudoGUI",elements.at(1));
                }
                else {
                    m_setupFile->setSectionValue("RequiredApplications",m_initParameters.at(m_initIndex),"n/a");
                    m_setupFile->setSectionValue("Settings","SudoGUI","n/a");
                    QMessageBox::warning(this, tr("ERROR"), tr("Neither 'pkexe' nor 'kdesu' os installed. Please install at least one of this applications!"), QMessageBox::Ok);
                    emit quitApplication();
                }
                break;
            case 7:
                if (rc != "")
                {
                    m_ui->applicationUserLabel->setText("Database Owner : " + elements.at(0));
                    m_ui->applicationUserStatusLabel->setPixmap(QPixmap(":/icons/icons/create.png"));
                    m_freshclamConf = new setupFileHandler(QDir::homePath()+ "/.clamav-gui/freshclam.conf");
                    m_freshclamConf->setSingleLineValue("DatabaseOwner",elements.at(0), "When started by root, drop privileges to a specified user. Default: vscan");
                    delete m_freshclamConf;
                    m_setupFile->setSectionValue("RequiredApplications","User",m_ui->applicationUserLabel->text().replace("Database Owner : ",""));
                }
                break;
            case 8:
                if (elements.count() > 0)
                {
                    m_ui->applicationGroupLabel->setText("Application Group : " + elements.at(0));
                    m_ui->applicationGroupStatusLabel->setPixmap(QPixmap(":/icons/icons/create.png"));
                    m_setupFile->setSectionValue("RequiredApplications","Group",m_ui->applicationGroupLabel->text().replace("Application Group : ",""));
                }
                break;
            case 9:
                QFile file(QDir::homePath() + "/.clamav-gui/clamd.conf.man");
                if (file.open(QIODevice::WriteOnly|QIODevice::Text))
                {
                    QTextStream stream(&file);
                    stream << rc;
                    Qt::endl(stream);
                    file.close();
                }
                break;
        }

        m_initIndex++;

        if (m_initIndex < m_initCommands.size())
        {
            m_processParameters.clear();
            if (m_initParameters.at(m_initIndex) != "")
            {
                m_processParameters << m_initParameters.at(m_initIndex);
                m_initProcess->start(m_initCommands.at(m_initIndex),m_processParameters);
            }
            else {
                m_processParameters << m_initParameters.at(m_initIndex);
                m_initProcess->start(m_initCommands.at(m_initIndex),QStringList());
            }
        }
    }
}

void firstRunWindow::slot_sddComboBoxChanged()
{
    if (m_processInit == false)
    {
        setupFileHandler * m_clamdConf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/clamd.conf");
        setupFileHandler * m_freshclamConf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/freshclam.conf");
        if (m_ui->signatureDatabaseDirectoryComboBox->currentText() == QDir::homePath() + "/.clamav-gui/signatures")
        {
            m_setupFile->setSectionValue("Directories", "LoadSupportedDBFiles", "checked|" + QDir::homePath() + "/.clamav-gui/signatures");
            m_setupFile->setSectionValue("FreshClam","runasroot",false);
            m_clamdConf->setSingleLineValue("DatabaseDirectory",QDir::homePath() + "/.clamav-gui/signatures");
            m_freshclamConf->setSingleLineValue("DatabaseDirectory", QDir::homePath() + "/.clamav-gui/signatures", "Path to a directory containing database files.  This directory must already exist, be an absolute path, be writeable by freshclam and readable by clamd/clamscan. Default: /var/lib/clamav");
        }
        else  {
            m_setupFile->setSectionValue("Directories", "LoadSupportedDBFiles", "checked|" + m_ui->signatureDatabaseDirectoryComboBox->currentText());
            m_setupFile->setSectionValue("FreshClam","runasroot",true);
            m_clamdConf->setSingleLineValue("DatabaseDirectory",m_ui->signatureDatabaseDirectoryComboBox->currentText());
            m_freshclamConf->setSingleLineValue("DatabaseDirectory",m_ui->signatureDatabaseDirectoryComboBox->currentText(),"Path to a directory containing database files.  This directory must already exist, be an absolute path, be writeable by freshclam and readable by clamd/clamscan. Default: /var/lib/clamav");
        }
        emit settingChanged();
        delete m_clamdConf;
        delete m_freshclamConf;
    }
}

void firstRunWindow::slot_languageChanged()
{
    m_setupFile->setSectionValue("Setup", "language", m_ui->applicationLanguageComboBox->currentText().mid(0, 7));
}

void firstRunWindow::slot_done()
{
    QString databaseDirectory = m_setupFile->getSectionValue("Directories","LoadSupportedDBFiles");
    databaseDirectory = databaseDirectory.mid(databaseDirectory.indexOf("|")+1);
    setupFileHandler * m_freshclamConf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/freshclam.conf");
    m_freshclamConf->setSingleLineValue("DatabaseDirectory",databaseDirectory);
    delete m_freshclamConf;
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

void firstRunWindow::createBaseDirStructure()
{
    //*****************************************************************************
    //creating basic Directory-Structur
    //*****************************************************************************
    QDir dir(QDir::homePath());
    dir.mkpath(QDir::homePath() + "/.clamav-gui");
    dir.mkpath(QDir::homePath() + "/.clamav-gui/logs");
    dir.mkpath(QDir::homePath() + "/.clamav-gui/profiles");
    dir.mkpath(QDir::homePath() + "/.clamav-gui/quarantine");
    dir.mkpath(QDir::homePath() + "/.clamav-gui/signatures");
    QFile::setPermissions(QDir::homePath() + "/.clamav-gui/signatures",
                          QFileDevice::ReadOwner  | QFileDevice::ReadUser  | QFileDevice::ReadGroup  | QFileDevice::ReadOther  |
                          QFileDevice::WriteOwner | QFileDevice::WriteUser | QFileDevice::WriteGroup | QFileDevice::WriteOther |
                          QFileDevice::ExeOwner   | QFileDevice::ExeUser   | QFileDevice::ExeGroup   | QFileDevice::ExeOther);
    //_____________________________________________________________________________________________________________________________________
    // For UALinux
    // If the settings.ini in the home folder of the user is not present a predefined version is been copied into the folder.
    //_____________________________________________________________________________________________________________________________________
    if ((!QFileInfo::exists(QDir::homePath() + "/.clamav-gui/settings.ini")) && (!QFile::exists(QDir::homePath() + "/.clamav-gui/settings.ini")))
    {
        dir.mkdir(QDir::homePath() + "/.clamav-gui");

        if (QFile::exists("/etc/clamav-gui/settings.ini"))
            QFile::copy("/etc/clamav-gui/settings.ini", QDir::homePath() + "/.clamav-gui/settings.ini");
    }
    //______________________________________________________________________________________________________________________________________

    m_ui->initialDirectoryStructureStatusLabel->setPixmap(QPixmap(":/icons/icons/create.png"));
}

void firstRunWindow::createServiceMenu()
{
    bool created = false;
    //*****************************************************************************
    //creating service Menu for Dolphin
    //*****************************************************************************
    QString serviceMenuPath;
    if (QFileInfo::exists(QDir::homePath() + "/.local/share/kservices5/ServiceMenus"))
        serviceMenuPath = QDir::homePath() + "/.local/share/kservices5/ServiceMenus";

    if (serviceMenuPath.isEmpty() && QFileInfo::exists(QDir::homePath() + "/.local/share/kio/servicemenus"))
        serviceMenuPath = QDir::homePath() + "/.local/share/kio/servicemenus";

    if ((serviceMenuPath.isEmpty()) && (QFileInfo::exists(QCoreApplication::applicationDirPath() + "/../share/" + "kio/servicemenues")))
        serviceMenuPath = QDir::homePath() + "/.local/share/kio/servicemenus";

    if ((serviceMenuPath.isEmpty()) && (QFileInfo::exists(QCoreApplication::applicationDirPath() + "/../share/" + "kservices5/ServiceMenus")))
        serviceMenuPath = QDir::homePath() + "/.local/share/kservices5/ServiceMenus";

    if (serviceMenuPath != "")
    {
        if (!QFileInfo::exists(serviceMenuPath))
        {
            QDir dir(serviceMenuPath);
            dir.mkpath(serviceMenuPath);
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

        QFile file(serviceMenuPath + "/scanWithClamAV-GUI.desktop");
        file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadGroup |
                            QFileDevice::WriteGroup | QFileDevice::ExeGroup);
        created = true;
    }
// Service Menu for NEMO
    if (QFileInfo::exists(QDir::homePath() + "/.local/share/nemo/actions"))
    {
        setupFileHandler* serviceFile = new setupFileHandler(QDir::homePath() + "/.local/share/nemo/actions/scan.nemo_action", this);
        serviceFile->setSectionValue("Nemo Action", "Name", "scan with ClamAV-GUI");
        serviceFile->setSectionValue("Nemo Action", "Comment", "scan with ClamAV-GUI");
        serviceFile->setSectionValue("Nemo Action", "Exec", "clamav-gui --scan &F");
        serviceFile->setSectionValue("Nemo Action", "Icon-Name", "clamav-gui");
        serviceFile->setSectionValue("Nemo Action", "Selection", "notnone");
        serviceFile->setSectionValue("Nemo Action", "Extensions", "any");
        serviceFile->setSectionValue("Nemo Action", "Separator", ",");
        serviceFile->setSectionValue("Nemo Action", "Dependencies", "clamav-gui");
        delete serviceFile;
    }

// ServiceMenu for GNOME-Commander
    QStringList gnomecommanderParams;

    m_gsettingsProcess = new QProcess(this);
    gnomecommanderParams << "get"  << "org.gnome.gnome-commander.preferences.general" << "favorite-apps";
    connect(m_gsettingsProcess,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slot_gsettingsProcessFinished(int,QProcess::ExitStatus)));
    m_gsettingsProcess->start("gsettings",gnomecommanderParams);

    if (created == true)
    {
        m_ui->dolphinContestMenuStatusLabel->setPixmap(QPixmap(":/icons/icons/create.png"));
    }
    else {
        m_ui->dolphinContestMenuStatusLabel->setPixmap(QPixmap(":/icons/icons/cancel.png"));
    }
    //*****************************************************************************

}

void firstRunWindow::createInitialSettings()
{
    QString virusDatabasePath = "";

    if (QFileInfo::exists("/var/lib/clamav")) m_ui->signatureDatabaseDirectoryComboBox->addItem("/var/lib/clamav");
    if (QFileInfo::exists("/usr/local/share/clamav")) m_ui->signatureDatabaseDirectoryComboBox->addItem("/usr/local/share/clamav");
    m_ui->signatureDatabaseDirectoryComboBox->addItem(QDir::homePath() + "/.clamav-gui/signatures");

    if (QFileInfo::exists("/var/lib/clamav") && (QFile::exists("/var/lib/clamav/freshclam.dat")))
    {
        virusDatabasePath = "/var/lib/clamav";
    }
    if ((QFileInfo::exists("/usr/local/share/clamav") && (QFile::exists("/usr/local/share/clamav/freshclam.dat"))))
    {
        virusDatabasePath = "/usr/local/share/clamav";
    }

    if (virusDatabasePath != "")
    {
        if (m_setupFile->keywordExists("Directories","LoadSupportedDBFiles") == false)
            m_setupFile->setSectionValue("Directories", "LoadSupportedDBFiles", "checked|" + virusDatabasePath);

        if (m_setupFile->keywordExists("FreshClam","runasroot") == false)
            m_setupFile->setSectionValue("FreshClam","runasroot",true);
    }
    else {
        if (m_setupFile->keywordExists("Directories","LoadSupportedDBFiles") == false)
            m_setupFile->setSectionValue("Directories", "LoadSupportedDBFiles", "checked|" + QDir::homePath() + "/.clamav-gui/signatures");

        m_setupFile->setSectionValue("FreshClam","runasroot",false);
        m_ui->signatureDatabaseDirectoryComboBox->setCurrentText(QDir::homePath() + "/.clamav-gui/signatures");
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

    if (m_setupFile->keywordExists("FreshClam","UpdatesPerDay") == false)
        m_setupFile->setSectionValue("FreshClam","UpdatesPerDay",12);

    if (m_setupFile->keywordExists("FreshClam","DataBaseToUpdate") == false)
        m_setupFile->setSectionValue("FreshClam","DataBaseToUpdate",0);

    if (m_setupFile->keywordExists("Freshclam","StartDaemon") == false)
        m_setupFile->setSectionValue("Freshclam","StartDaemon",false);

    if (m_setupFile->keywordExists("Setup","DisableLogHighlighter") == false)
        m_setupFile->setSectionValue("Setup","DisableLogHighlighter",false);

    if (m_setupFile->keywordExists("Setup","WindowState") == false)
        m_setupFile->setSectionValue("Setup","WindowState","maximized");

    m_ui->initialSettingsFileStatusLabel->setPixmap(QPixmap(":/icons/icons/create.png"));
}

void firstRunWindow::createClamdConfFile()
{
    QFile clamdConfFile(QDir::homePath() + "/.clamav-gui/clamd.conf");

    if (clamdConfFile.exists() == false)
    {
        m_clamdConf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/clamd.conf", this);
        QString value = m_setupFile->getSectionValue("Directories", "LoadSupportedDBFiles");

        if (value.indexOf("checked|") == 0)
            m_clamdConf->addSingleLineValue("DatabaseDirectory", value.mid(value.indexOf("|") + 1));

        m_clamdConf->setSingleLineValue("LogSyslog", "no");
        m_clamdConf->setSingleLineValue("LogFacility", "LOG_LOCAL6");
        m_clamdConf->setSingleLineValue("PidFile", "/tmp/clamd.pid","By default clamd binds to INADDR_ANY. This option allows you to restrict the TCP address and provide some degree of protection from the outside world. This option can be specified multiple times in order to listen on multiple IPs. IPv6 is now supported. Default: disabled");
        m_clamdConf->setSingleLineValue("ExtendedDetectionInfo", "yes");
        m_clamdConf->setSingleLineValue("LocalSocket", QDir::homePath() + "/.clamav-gui/clamd-socket");
        m_clamdConf->setSingleLineValue("LogFile", QDir::homePath() + "/.clamav-gui/clamd.log","Save all reports to a log file. Default: disabled");
        m_clamdConf->setSingleLineValue("LocalSocketGroup", "users");
        m_clamdConf->setSingleLineValue("TCPAddr", "127.0.0.1","By default clamd binds to INADDR_ANY. This option allows you to restrict the TCP address and provide some degree of protection from the outside world. This option can be specified multiple times in order to listen on multiple IPs. IPv6 is now supported. Default: disabled");
        m_clamdConf->addSingleLineValue("TCPAddr", "::1","By default clamd binds to INADDR_ANY. This option allows you to restrict the TCP address and provide some degree of protection from the outside world. This option can be specified multiple times in order to listen on multiple IPs. IPv6 is now supported. Default: disabled");
        m_clamdConf->setSingleLineValue("LogFileMaxSize", "1M");
        m_clamdConf->setSingleLineValue("LogTime", "no");
        m_clamdConf->setSingleLineValue("LogRotate", "yes");
        m_clamdConf->setSingleLineValue("OnAccessMaxFileSize", "10M");
        m_clamdConf->setSingleLineValue("OnAccessMaxThreads", "10");
        m_clamdConf->setSingleLineValue("OnAccessPrevention", "yes");
        m_clamdConf->setSingleLineValue("OnAccessDenyOnError", "no");
        m_clamdConf->setSingleLineValue("OnAccessExtraScanning", "yes");
        m_clamdConf->setSingleLineValue("OnAccessRetryAttempts", "0");
        m_clamdConf->setSingleLineValue("OnAccessExcludeUname", "root","This option allows exclusions via user names when using the on- access scanning client. It can be used multiple times, and has the same potential race condition limitations of the OnAccessEx‐ cludeUID option. Default: disabled");
        m_clamdConf->setSingleLineValue("OnAccessExcludeUID", "0","With this option you can exclude specific UIDs. Processes with these UIDs will be able to access all files without triggering scans or permission denied events. This option can be used multiple times (one per line). Note: using a value of 0 on any line will disable this option en‐ tirely. To exclude the root UID (0) please enable the OnAccessEx‐ cludeRootUID option. Also note that if clamd cannot check the uid of the process that generated an on-access scan event (e.g., because OnAccessPreven‐ tion was not enabled, and the process already exited), clamd will perform a scan. Thus, setting OnAccessExcludeUID is not guaran‐ teed to prevent every access by the specified uid from triggering a scan (unless OnAccessPrevention is enabled). Default: disabled");
    }

    m_ui->clamdConfStatusLabel->setPixmap(QPixmap(":/icons/icons/create.png"));
}

void firstRunWindow::createFreshclamConfFile()
{
    if (QFile::exists(QDir::homePath() + "/.clamav-gui/freshclam.conf") == false)
    {
        QFile freshclamConfFile(QDir::homePath() + "/.clamav-gui/freshclam.conf");
        freshclamConfFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);

        m_freshclamConf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/freshclam.conf", this);
        QString value = m_setupFile->getSectionValue("Directories", "LoadSupportedDBFiles");
        if (value.indexOf("checked|") == 0)
            m_freshclamConf->setSingleLineValue("DatabaseDirectory", value.mid(value.indexOf("|") + 1),"Path to a directory containing database files.  This directory must already exist, be an absolute path, be writeable by freshclam and readable by clamd/clamscan. Default: /var/lib/clamav");

        m_freshclamConf->setSingleLineValue("LogSyslog", "no", "Enable logging to Syslog. May be used in combination with UpdateLogFile. Default: disabled.");
        m_freshclamConf->setSingleLineValue("LogFacility", "LOG_LOCAL6", "Specify the type of syslog messages - please refer to 'man syslog' for facility names. Default: LOG_LOCAL6");
        m_freshclamConf->setSingleLineValue("LogTime", "no", "Log time with each message. Default: no");
        m_freshclamConf->setSingleLineValue("LogRotate", "no", "Rotate log file. Requires LogFileMaxSize option set prior to this option. Default: no");
        m_freshclamConf->setSingleLineValue("PidFile", "/tmp/freshclam.pid", "Write the daemon's pid to the specified file. Default: disabled");
        m_freshclamConf->setSingleLineValue("DatabaseOwner", "clamav", "When started by root, drop privileges to a specified user. Default: vscan");
        m_freshclamConf->setSingleLineValue("DatabaseMirror", "database.clamav.net", "DatabaseMirror  specifies  to  which  mirror(s) freshclam should connect. You should have at least one entries: database.clamav.net.  Now that CloudFlare is being used as our Content Delivery Network (CDN), this one domain name works world-wide to direct freshclam to the closest geographic endpoint. Default: database.clamav.net");
        m_freshclamConf->setSingleLineValue("LogVerbose", "no", "Enable verbose logging. Default: disabled");
        m_freshclamConf->setSingleLineValue("Checks", "12", "Number of database checks per day. Default: 12");
    }
}

