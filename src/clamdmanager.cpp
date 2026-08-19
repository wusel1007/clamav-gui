#include "clamdmanager.h"
#define css_red "background-color:red;color:yellow"
#define css_green "background-color:green;color:yellow"
#define css_mono "background-color:#404040;color:white"

clamdManager::clamdManager(QWidget* parent, setupFileHandler* setupFile) : QWidget(parent), m_setupFile(setupFile)
{
    m_ui.setupUi(this);
    m_startup = true;
    m_monochrome = false;
    m_waitForFreshclam = true;
    m_clamdStartupCounter = 0;
    m_clamdManagerLocked = false;
    m_startDelayTimer = new QTimer(this);
    m_startDelayTimer->setSingleShot(true);
    connect(m_startDelayTimer, SIGNAL(timeout()), this, SLOT(slot_startDelayTimerExpired()));

    m_processWatcher = new QTimer(this);
    connect(m_processWatcher, SIGNAL(timeout()), this, SLOT(slot_processWatcherExpired()));
    m_processWatcher->start(30000);

    m_getClamdConfParametersProcess = new QProcess(this);
    connect(m_getClamdConfParametersProcess,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slot_getClamdConfParameterProcessFinished()));

    m_findClamdProcess = new QProcess(this);
    connect(m_findClamdProcess,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slot_findClamdProcessFinished()));

    m_clamdLogWatcher = new QFileSystemWatcher(this);
    connect(m_clamdLogWatcher,SIGNAL(fileChanged(QString)),this,SLOT(slot_logFileContentChanged()));

    if (pidof("clamd") != "")
        slot_findClamdProcessFinished();
    else {
        m_setupFile->setSectionValue("Clamd","ClamdPid","n/a");
        m_setupFile->setSectionValue("Clamd","ClamonaccPid","n/a");
        m_setupFile->setSectionValue("Clamd","Status","not running");
        m_setupFile->setSectionValue("Clamd","Status2","not running");
        m_setupFile->setSectionValue("Clamd","ClamdPidFile","/tmp/clamd.pid");
        m_setupFile->setSectionValue("Clamd","ClamdConfPath",QDir::homePath() + "/.clamav-gui/clamd.conf");
        m_setupFile->setSectionValue("Clamd","ClamdLogFile",QDir::homePath() + "/.clamav-gui/clamd.log");
    }
}

QString clamdManager::trimLocationOutput(QString value)
{
    QString rc = value;
    int start = rc.indexOf(" ") + 1;
    int ende = rc.indexOf(" ", start);

    ende = ende - start;
    rc = value.mid(start, ende);
    rc = rc.replace("\n", "");
    return rc;
}

void clamdManager::slot_initClamdSettings()
{
    m_initprocessrunning = true;
    m_dirsUnderMonitoring = 0;
    m_logHighlighter = NULL;

    m_clamdLocation = m_setupFile->getSectionValue("Clamd","ClamdLocation");
    m_clamonaccLocation = m_setupFile->getSectionValue("Clamd","ClamonaccLocation");
    m_sudoGUI = m_setupFile->getSectionValue("Settings", "SudoGUI");

    m_clamdConf = new setupFileHandler(m_setupFile->getSectionValue("Clamd","ClamdConfPath"), this);

    QStringList monitorings = m_setupFile->getKeywords("Clamonacc");
    m_dirsUnderMonitoring = monitorings.length();

    m_ui.monitoringComboBox->addItems(monitorings);

    m_clamdPidWatcher = new QFileSystemWatcher(this);
    connect(m_clamdPidWatcher, SIGNAL(fileChanged(QString)), this, SLOT(slot_pidWatcherTriggered()));

    m_startClamdProcess = new QProcess(this);
    connect(m_startClamdProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slot_startClamdProcessFinished(int, QProcess::ExitStatus)));

    m_killProcess = new QProcess(this);
    connect(m_killProcess, SIGNAL(finished(int)), this, SLOT(slot_killClamdProcessFinished()));

    m_findclamonaccProcess = new QProcess(this);
    connect(m_findclamonaccProcess, SIGNAL(finished(int)), this, SLOT(slot_findclamonaccProcessFinished(int)));

    m_restartClamonaccProcess = new QProcess(this);
    connect(m_restartClamonaccProcess, SIGNAL(finished(int)), this, SLOT(slot_restartClamonaccProcessFinished()));

    if (checkClamdRunning() == true)
    {
        /*m_ui.startStopClamdPushButton->setStyleSheet(selectColor("green"));
        m_ui.startStopClamdPushButton->setText(tr("  Clamd running - Stop clamd"));
        m_ui.startStopClamdPushButton->setIcon(QIcon(":/icons/icons/stopclamd.png"));*/
        if (QFileInfo::exists(m_setupFile->getSectionValue("Clamd","ClamdPidFile")) == true)
            m_clamdPidWatcher->addPath(m_setupFile->getSectionValue("Clamd","ClamdPidFile"));
    }
/*    else {
        m_ui.startStopClamdPushButton->setStyleSheet(selectColor("red"));
    }*/

    m_clamdRestartInProgress = false;

    m_ui.restartClamdPushButton->setVisible(false);
    slot_updateClamdConf();

    getClamdConfElements();

    m_initprocessrunning = false;
}

void clamdManager::slot_dbPathChanged(QString dbPath)
{
    if (m_clamdManagerLocked == false)
    {
        if ((m_clamdConf->getSingleLineValue("DatabaseDirectory") != dbPath) && (m_clamdConf->getSingleLineValue("DatabaseDirectory") != ""))
        {
            foreach(ClamdConfOptionBaseClass* item,m_clamdConfParameters)
            {
                if (item->getKeyword() == "DatabaseDirectory") item->setValue(dbPath);
            }
            m_clamdConf->setSingleLineValue("DatabaseDirectory", dbPath);
            slot_clamdSettingsChanged();
        }
    }
}

void clamdManager::slot_updateClamdConfParameters()
{
    foreach(ClamdConfOptionBaseClass* item,m_clamdConfParameters)
    {
        m_ui.layout1->removeWidget(item);
        delete item;
    }
    m_clamdConfParameters.clear();

    QStringList parameters;
    parameters << "clamd.conf";
    startProcess(m_getClamdConfParametersProcess,"man",parameters);
}

void clamdManager::slot_getClamdConfParameterProcessFinished()
{
    QString rc = m_getClamdConfParametersProcess->readAll();
    QFile file(QDir::homePath() + "/.clamav-gui/clamd.conf.man");
    if (file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QTextStream stream(&file);
        stream << rc;
        Qt::endl(stream);
        file.close();
    }
    getClamdConfElements();
}

void clamdManager::slot_filterChanged(QString searchString)
{
    QString keyword;
    QString label;

    if ((m_ui.showSelectedCheckBox->isChecked()) || (m_ui.showUnselectedCheckBox->isChecked()))
    {
        if (m_ui.showSelectedCheckBox->isChecked())
        {
            foreach (ClamdConfOptionBaseClass* item, m_clamdConfParameters)
            {
                item->setVisible(item->isChecked());
            }
        }
        else {
            foreach (ClamdConfOptionBaseClass* item, m_clamdConfParameters)
            {
                item->setVisible(!item->isChecked());
            }
        }
    }
    else {
        if (searchString != "")
        {
            searchString = searchString.toUpper();

            foreach(ClamdConfOptionBaseClass* item,m_clamdConfParameters)
            {
                keyword = item->getKeyword().toUpper();
                label = item->getLabel().toUpper();
                ((keyword.indexOf(searchString) != -1) || (label.indexOf(searchString) != -1))?item->setVisible(true):item->setVisible(false);
            }
        }
        else {
            foreach (ClamdConfOptionBaseClass* item, m_clamdConfParameters)
            {
                item->setVisible(true);
            }
        }
    }
}

void clamdManager::slot_showSelectedChecked()
{
    if (m_ui.showSelectedCheckBox->isChecked())
        m_ui.showUnselectedCheckBox->setChecked(false);

    slot_filterChanged(m_ui.lineEdit->text());
}

void clamdManager::slot_showUnselectedChecked()
{
    if (m_ui.showUnselectedCheckBox->isChecked())
        m_ui.showSelectedCheckBox->setChecked(false);

    slot_filterChanged(m_ui.lineEdit->text());
}

void clamdManager::slot_findClamdProcessFinished()
{
    bool clamdConfPathFlag = false;
    QString clamdConfPath;
    QString processStatus = runProg("bash",{"-c","ps ax | grep clamd"});
    QStringList processes = processStatus.split("\n");
    processStatus = processes.at(0);
    while (processStatus.indexOf("  ") != -1)
        processStatus = processStatus.replace("  "," ");
    QStringList lines = processStatus.split(" ");

    m_setupFile->setSectionValue("Clamd","ClamdPid",pidof("clamd"));
    m_setupFile->setSectionValue("Clamd","Status","is running");

    foreach (QString field, lines)
    {
        field = field.trimmed();

        if (clamdConfPathFlag)
        {
            clamdConfPath = field;
            m_setupFile->setSectionValue("Clamd","ClamdConfPath",clamdConfPath);
            QFileInfo info(clamdConfPath);
            if (info.isWritable() == false)
            {
                m_ui.tabWidget->setDisabled(true);
                m_clamdManagerLocked = true;
            }
            else
                m_ui.tabWidget->setDisabled(false);

            setupFileHandler m_confSetupFile(field);

            if (m_confSetupFile.singleLineExists("ClamdPidFile"))
                m_setupFile->setSectionValue("Clamd","ClamdPidFile",m_confSetupFile.getSingleLineValue("ClamdPidFile"));
            else
                m_setupFile->setSectionValue("Clamd","ClamdPidFile","/tmp/clamd.pid");

            if (m_confSetupFile.singleLineExists("LogFile"))
                m_setupFile->setSectionValue("Clamd","ClamdLogFile",m_confSetupFile.singleLineExists("LogFile"));
            else {
                m_setupFile->setSectionValue("Clamd","ClamdLogFile","");
                m_ui.clamdLogPlainTextEdit->clear();
            }

            clamdConfPathFlag = false;
        }

        if (field == "-c")
        {
            clamdConfPathFlag = true;
        }
    }

    if (QFileInfo::exists(clamdConfPath))
    {
        setupFileHandler clamdConfHandler(clamdConfPath);
        if (clamdConfHandler.singleLineExists("LogFile"))
            m_setupFile->setSectionValue("Clamd","ClamdLogFile",clamdConfHandler.getSingleLineValue("LogFile"));
    }

    emit slot_restartClamonaccProcessFinished();
}

void clamdManager::slot_updateClamdConf()
{
    QString logPath = m_setupFile->getSectionValue("Clamd","ClamdLogFile");
    if (logPath != "")
    {
        QFile checkFile(logPath);

        if (checkFile.exists() == false)
        {
            if (checkFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream stream(&checkFile);
                stream << "\n";
                checkFile.close();
            }
        }
        else {
            slot_logFileContentChanged();
        }

        if (m_clamdLogWatcher->directories().size() > 0)
            m_clamdLogWatcher->removePaths(m_clamdLogWatcher->directories());
        if (QFileInfo::exists(logPath))
            m_clamdLogWatcher->addPath(logPath);
    }
    if (m_clamdManagerLocked == false)
    {

        m_freshclamConf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/freshclam.conf", this);

        QStringList watchList = m_setupFile->getKeywords("Clamonacc");
        foreach (QString entry, watchList)
        {
            m_clamdConf->addSingleLineValue("OnAccessIncludePath", entry);
        }

        if ((m_setupFile->sectionExists("REGEXP_and_IncludeExclude")) &&
            (m_setupFile->getSectionValue("REGEXP_and_IncludeExclude", "DontScanDirectoriesMatchingRegExp").indexOf("checked|") == 0))
            m_clamdConf->addSingleLineValue("ExcludePath",
                                            m_setupFile->getSectionValue("REGEXP_and_IncludeExclude", "DontScanDirectoriesMatchingRegExp").mid(8));

        if ((m_setupFile->sectionExists("REGEXP_and_IncludeExclude")) &&
            (m_setupFile->getSectionValue("REGEXP_and_IncludeExclude", "DontScanFileNamesMatchingRegExp").indexOf("checked|") == 0))
            m_clamdConf->addSingleLineValue("ExcludePath",
                                            m_setupFile->getSectionValue("REGEXP_and_IncludeExclude", "DontScanFileNamesMatchingRegExp").mid(8));

        if (m_setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude", "EnablePUAOptions") == true)
        {
            QStringList PUAKeywords;
            PUAKeywords << "LoadPUAPacked" << "LoadPUAPWTool" << "LoadPUANetTool" << "LoadPUAP2P" << "LoadPUAIRC" << "LoadPUARAT" << "LoadPUANetToolSpy"
                        << "LoadPUAServer";
            PUAKeywords << "LoadPUAScript" << "LoadPUAAndr" << "LoadPUAJava" << "LoadPUAOsx" << "LoadPUATool" << "LoadPUAUnix" << "LoadPUAWin";

            QStringList PUASwitches;
            PUASwitches << "Packed" << "PWTool" << "NetTool" << "P2P" << "IRC" << "RAT" << "NetToolSpy" << "Server" << "Script" << "Andr" << "Java"
                        << "Osx" << "Tool" << "Unix" << "Win";

            for (int i = 0; i < PUAKeywords.length(); i++)
            {
                if (m_setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude", PUAKeywords.at(i)) == true)
                    m_clamdConf->addSingleLineValue("IncludePUA", PUASwitches.at(i));
            }
        }
    }
}

void clamdManager::slot_logFileContentChanged()
{
    if (m_setupFile->getSectionValue("Clamd","ClamdLogFile") != "")
    {
        QFile file(m_setupFile->getSectionValue("Clamd","ClamdLogFile"));
        QString content;
        QString checkString;

        if (file.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&file);
            content = stream.readAll();
            file.close();
        }

        QStringList lines = content.split("\n");
        foreach (QString line, lines)
        {
            if ((line.indexOf("/") == 0) && (line.indexOf("FOUND") == line.length() - 5))
                checkString = line;
        }

        if ((checkString != m_lastFound) && (checkString != ""))
        {
            m_lastFound = checkString;
            emit setBallonMessage(2, tr("WARNING"), m_lastFound);
        }

        m_ui.clamdLogPlainTextEdit->clear();
        m_ui.clamdLogPlainTextEdit->insertPlainText(content);
        m_ui.clamdLogPlainTextEdit->ensureCursorVisible();
    }
}

void clamdManager::slot_clamdStartStopButtonClicked()
{
    QStringList monitorings = m_setupFile->getKeywords("Clamonacc");
    QStringList parameters;
    QString clamonaccOptions;

    if (m_setupFile->getSectionValue("Directories", "MoveInfectedFiles").indexOf("checked") == 0)
    {
        QString path = m_setupFile->getSectionValue("Directories", "MoveInfectedFiles");
        path = path.mid(path.indexOf("|") + 1);
        clamonaccOptions = " --move=" + path;
    }

    if (m_setupFile->getSectionValue("Directories", "CopyInfectedFiles").indexOf("checked") == 0)
    {
        QString path = m_setupFile->getSectionValue("Directories", "CopyInfectedFiles");
        path = path.mid(path.indexOf("|") + 1);
        clamonaccOptions = clamonaccOptions + " --copy=" + path;
    }

    if (m_setupFile->getSectionValue("Directories", "ScanReportToFile").indexOf("checked") == 0)
    {
        QString path = m_setupFile->getSectionValue("Directories", "ScanReportToFile");
        path = path.mid(path.indexOf("|") + 1);
        clamonaccOptions = clamonaccOptions + " --log=" + path;
    }

    if (m_setupFile->keywordExists("Directories", "--verbose") == true)
        clamonaccOptions += " --verbose";
    if (m_setupFile->keywordExists("SelectedOptions", "--remove<equal>yes") == true)
        clamonaccOptions += "--remove";
    if (m_setupFile->keywordExists("SelectedOptions", "--allmatch<equal>yes") == true)
        clamonaccOptions += "--allmatch";

    if (checkClamdRunning() == false)
    {
        if (QFileInfo::exists(m_setupFile->getSectionValue("Clamd","ClamdPidFile")))
        {
            QFile pidFile(m_setupFile->getSectionValue("Clamd","ClamdPidFile"));
            pidFile.remove();
        }
        slot_updateClamdConf();

        m_ui.clamdIconLabel->setMovie(new QMovie(":/icons/icons/gifs/spinning_segments_small.gif"));
        m_ui.clamdIconLabel->movie()->start();

        if (m_setupFile->getSectionValue("Clamd","ClamdLogFile") != "")
        {
            if (m_clamdLogWatcher->directories().size() > 0)
                m_clamdLogWatcher->removePaths(m_clamdLogWatcher->directories());

            QString logPath = m_setupFile->getSectionValue("Clamd","ClamdLogFile");
            if (logPath != "")
            {
                QFile logFile(logPath);
                /*if (logFile.exists() == true)
                logFile.remove();*/
                if (logFile.open(QIODevice::ReadWrite))
                {
                    QTextStream stream(&logFile);
                    stream << "\n";
                    logFile.close();
                }
                if (QFileInfo::exists(logPath))
                    m_clamdLogWatcher->addPath(logPath);
            }
        }
        m_ui.clamdLogPlainTextEdit->clear();
        m_ui.startStopClamdPushButton->setText(tr("  Clamd starting. Please wait!"));

        QFile startclamdFile(QDir::homePath() + "/.clamav-gui/startclamd.sh");
        if (startclamdFile.exists() == true)
            startclamdFile.remove();
        if (startclamdFile.open(QIODevice::Text | QIODevice::ReadWrite))
        {
            // QString logFile = m_clamdConf->getSingleLineValue("LogFile");
            QTextStream stream(&startclamdFile);
            if (monitorings.length() > 0)
            {
                stream << "#!/bin/bash\n"
                       << m_clamdLocation + " 2> " + m_setupFile->getSectionValue("Clamd","ClamdLogFile") + " -c " + m_setupFile->getSectionValue("Clamd","ClamdConfPath") +
                              " && " + m_clamonaccLocation + " -c " + m_setupFile->getSectionValue("Clamd","ClamdConfPath") + " -l " +
                              m_setupFile->getSectionValue("Clamd","ClamdLogFile") + clamonaccOptions;
            }
            else {
                stream << "#!/bin/bash\n" << m_clamdLocation + " 2> " + m_setupFile->getSectionValue("Clamd","ClamdLogFile") + " -c " + m_setupFile->getSectionValue("Clamd","ClamdConfPath");
                m_setupFile->setSectionValue("Clamd", "Status2", "n/a");
            }
            startclamdFile.close();
            startclamdFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadGroup |
                                          QFileDevice::WriteGroup | QFileDevice::ExeGroup);
        }
        parameters << QDir::homePath() + "/.clamav-gui/startclamd.sh";
        if (m_sudoGUI == "")
            m_sudoGUI = m_setupFile->getSectionValue("Settings", "SudoGUI");
        startProcess(m_startClamdProcess,m_sudoGUI, parameters);
        m_setupFile->setSectionValue("Clamd", "Status", "starting up ...");
        if (m_dirsUnderMonitoring > 0)
            m_setupFile->setSectionValue("Clamd", "Status2", "starting up ...");
        emit systemStatusChanged();
    }
    else {
        m_setupFile->setSectionValue("Clamd", "Status", "shutting down ...");
        emit systemStatusChanged();
        m_ui.startStopClamdPushButton->setText(tr("  Stopping Clamd. Please wait!"));
        QFile stopclamdFile(QDir::homePath() + "/.clamav-gui/stopclamd.sh");
        if (stopclamdFile.exists() == true)
            stopclamdFile.remove();
        if (stopclamdFile.open(QIODevice::Text | QIODevice::ReadWrite))
        {
            QTextStream stream(&stopclamdFile);
            if (m_setupFile->getSectionValue("Clamd","ClamonaccPid") != "n/a")
            {
                stream << "#!/bin/bash\n/bin/kill -9 " + m_setupFile->getSectionValue("Clamd","ClamdPid") + " && kill -9 " + m_setupFile->getSectionValue("Clamd","ClamonaccPid");
            }
            else {
                stream << "#!/bin/bash\n/bin/kill -9 " + m_setupFile->getSectionValue("Clamd","ClamdPid");
            }
            stopclamdFile.close();
            stopclamdFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadGroup |
                                         QFileDevice::WriteGroup | QFileDevice::ExeGroup);
        }
        parameters << QDir::homePath() + "/.clamav-gui/stopclamd.sh";
        if (m_sudoGUI == "")
            m_sudoGUI = m_setupFile->getSectionValue("Settings", "SudoGUI");
        startProcess(m_killProcess,m_sudoGUI, parameters);
    }
    m_ui.startStopClamdPushButton->setEnabled(false);
    m_ui.monitoringAddButton->setEnabled(false);
    m_ui.monitoringDelButton->setEnabled(false);
}

void clamdManager::slot_startClamdProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if ((exitCode != 0) || (exitStatus == QProcess::CrashExit))
        m_clamdStartupCounter = 0;
    m_ui.clamdIconLabel->clear();
    m_ui.clamdIconLabel->setPixmap(QPixmap(":/icons/icons/onaccess.png"));
    m_ui.clamdIconLabel_2->clear();
    ;
    m_ui.clamdIconLabel_2->setPixmap(QPixmap(":/icons/icons/options.png"));

    if (checkClamdRunning() == false)
    {
        if (m_clamdPidWatcher->directories().size() > 0)
            m_clamdPidWatcher->removePaths(m_clamdPidWatcher->directories());
        m_ui.startStopClamdPushButton->setStyleSheet(selectColor("red"));
        m_ui.startStopClamdPushButton->setText(tr("  Clamd not running - Start Clamd"));
        m_ui.startStopClamdPushButton->setIcon(QIcon(":/icons/icons/startclamd.png"));

        m_setupFile->setSectionValue("Clamd", "ClamdPid", "n/a");
        m_setupFile->setSectionValue("Clamd", "ClamonaccPid", "n/a");
        m_setupFile->setSectionValue("Clamd", "Status", "not running");

        emit systemStatusChanged();

        if (m_clamdStartupCounter > 0)
        {
            if (m_waitForFreshclam == false)
                m_clamdStartupCounter--;
            m_startDelayTimer->start(2500);
        }
    }
    else {
        m_clamdStartupCounter = 0;

        if (m_clamdPidWatcher->directories().size() > 0)
            m_clamdPidWatcher->removePaths(m_clamdPidWatcher->directories());
        if (QFileInfo::exists(m_setupFile->getSectionValue("Clamd","ClamdPidFile")))
            m_clamdPidWatcher->addPath(m_setupFile->getSectionValue("Clamd","ClamdPidFile"));

        startProcess(m_findClamdProcess,"bash",QStringList() << "-c" << "ps -ax | grep `pidof clamd` | grep clamd");

        emit systemStatusChanged();
        /*m_ui.startStopClamdPushButton->setStyleSheet(selectColor("green"));
        m_ui.startStopClamdPushButton->setText(tr("  Clamd running - Stop Clamd"));
        m_ui.startStopClamdPushButton->setIcon(QIcon(":/icons/icons/stopclamd.png"));*/

        slot_logFileContentChanged();

        QStringList parameters;
        parameters << "-s" << "clamonacc";
        startProcess(m_findclamonaccProcess,"pidof", parameters);
    }

    m_clamdRestartInProgress = false;

    m_ui.startStopClamdPushButton->setEnabled(true);
    m_ui.monitoringAddButton->setEnabled(true);
    m_ui.monitoringDelButton->setEnabled(true);
    m_ui.restartClamdPushButton->setVisible(false);
}

void clamdManager::slot_killClamdProcessFinished()
{
    m_ui.clamdIconLabel->clear();
    m_ui.clamdIconLabel->setPixmap(QPixmap(":/icons/icons/onaccess.png"));
    m_ui.clamdIconLabel_2->clear();
    ;
    m_ui.clamdIconLabel_2->setPixmap(QPixmap(":/icons/icons/options.png"));

    if (checkClamdRunning() == false)
    {
        if (m_clamdPidWatcher->directories().size() > 0)
            m_clamdPidWatcher->removePaths(m_clamdPidWatcher->directories());
        /*m_ui.startStopClamdPushButton->setStyleSheet(selectColor("red"));
        m_ui.startStopClamdPushButton->setText(tr("  Clamd not running - Start Clamd"));
        m_ui.startStopClamdPushButton->setIcon(QIcon(":/icons/icons/startclamd.png"));*/

        m_setupFile->setSectionValue("Clamd", "ClamdPid", "n/a");
        m_setupFile->setSectionValue("Clamd", "ClamonaccPid", "n/a");
        m_setupFile->setSectionValue("Clamd", "Status", "shut down");
        if (m_dirsUnderMonitoring > 0)
            m_setupFile->setSectionValue("Clamd", "Status2", "shut down");
        // reset to standard application values
        m_setupFile->setSectionValue("Clamd","ClamdConfPath",QDir::homePath() + "/.clamav-gui/clamd.conf");
        m_setupFile->setSectionValue("Clamd","ClamdLogFile",QDir::homePath() + "/.clamav-gui/clamd.log");
        m_setupFile->setSectionValue("Clamd","ClamdPidFile","/tmp/clamd.pid");
        emit systemStatusChanged();
    }
    else {
        if (QFileInfo::exists(m_setupFile->getSectionValue("Clamd","ClamdPidFile")) == true)
        {
            if (m_clamdPidWatcher->directories().size() > 0)
                m_clamdPidWatcher->removePaths(m_clamdPidWatcher->directories());
            if (QFileInfo::exists(m_setupFile->getSectionValue("Clamd","ClamdPidFile")))
                m_clamdPidWatcher->addPath(m_setupFile->getSectionValue("Clamd","ClamdPidFile"));
            /*m_ui.startStopClamdPushButton->setStyleSheet(selectColor("green"));
            m_ui.startStopClamdPushButton->setText(tr("  Clamd running - Stop Clamd"));
            m_ui.startStopClamdPushButton->setIcon(QIcon(":/icons/icons/stopclamd.png"));*/
        }
    }

    m_ui.startStopClamdPushButton->setEnabled(true);
    m_ui.monitoringAddButton->setEnabled(true);
    m_ui.monitoringDelButton->setEnabled(true);
}

void clamdManager::slot_findclamonaccProcessFinished(int rc)
{
    m_clamonaccPid = pidof("clamonacc");

    if (m_clamonaccPid != "")
    {
        m_setupFile->setSectionValue("Clamd", "ClamonaccPid", m_clamonaccPid);
        m_setupFile->setSectionValue("Clamd","Status2","is running");
        emit systemStatusChanged();
    }
    else {
        m_setupFile->setSectionValue("Clamd", "ClamonaccPid", "n/a");
        m_setupFile->setSectionValue("Clamd","Status2","not running");
        emit systemStatusChanged();
    }

    if ((m_setupFile->keywordExists("Clamd", "StartClamdOnStartup") == true) &&
        (m_setupFile->getSectionBoolValue("Clamd", "StartClamdOnStartup") == true))
    {
        m_ui.startClamdOnStartupCheckBox->setChecked(m_setupFile->getSectionBoolValue("Clamd", "StartClamdOnStartup"));
    }
}

void clamdManager::slot_startDelayTimerExpired()
{
    if (checkClamdRunning() == false)
    {
        emit setActiveTab();
        slot_clamdStartStopButtonClicked();
    }
}

void clamdManager::slot_waitForFreshclamStarted()
{
    m_waitForFreshclam = false;

    if ((m_startup == true) && (m_setupFile->keywordExists("Clamd", "StartClamdOnStartup") == true) &&
        (m_setupFile->getSectionBoolValue("Clamd", "StartClamdOnStartup") == true))
    {
        m_ui.startClamdOnStartupCheckBox->setChecked(m_setupFile->getSectionBoolValue("Clamd", "StartClamdOnStartup"));

        m_clamdStartupCounter = 5;
        if (m_startup == true)
            m_startDelayTimer->start(2500);
        m_startup = false;
    }
}

void clamdManager::slot_processWatcherExpired()
{
    QString clamdPid = m_setupFile->getSectionValue("Clamd", "ClamdPid");
    QString clamonaccPid = m_setupFile->getSectionValue("Clamd", "ClamonaccPid");

    QDir checkDir;
    if (clamdPid != "n/a")
    {
        if (checkDir.exists("/proc/" + clamdPid) == false)
        {
            m_setupFile->setSectionValue("Clamd", "ClamdPid", "n/a");

            emit systemStatusChanged();

            slot_killClamdProcessFinished();
        }
    }

    if (clamonaccPid != "n/a")
    {
        if (checkDir.exists("/proc/" + clamonaccPid) == false)
        {
            m_setupFile->setSectionValue("Clamd", "ClamonaccPid", "n/a");
            emit systemStatusChanged();
        }
    }
}

void clamdManager::slot_add_remove_highlighter(bool state)
{
    if (state == true)
    {
        if (m_logHighlighter != NULL)
        {
            delete m_logHighlighter;
            m_logHighlighter = NULL;
        }
        m_monochrome = true;
    }
    else {
        if (m_logHighlighter == NULL)
        {
            m_logHighlighter = new highlighter(m_ui.clamdLogPlainTextEdit->document());
        }
        else {
            delete m_logHighlighter;
            m_logHighlighter = new highlighter(m_ui.clamdLogPlainTextEdit->document());
        }
        m_monochrome = false;
    }
    checkClamdRunning();
}

void clamdManager::slot_pidWatcherTriggered()
{
    QFile pidFile(m_setupFile->getSectionValue("Clamd","ClamdPidFile"));
    if ((pidFile.exists() == false) && (m_clamdRestartInProgress == false))
    {
        if (m_clamdPidWatcher->directories().size() > 0)
            m_clamdPidWatcher->removePaths(m_clamdPidWatcher->directories());
        m_ui.startStopClamdPushButton->setStyleSheet(selectColor("red"));
        m_ui.startStopClamdPushButton->setText(tr("  Clamd not running - Start Clamd"));
        m_ui.startStopClamdPushButton->setIcon(QIcon(":/icons/icons/startclamd.png"));
        m_ui.startStopClamdPushButton->setEnabled(true);
        m_ui.monitoringAddButton->setEnabled(true);
        m_ui.monitoringDelButton->setEnabled(true);

        m_setupFile->setSectionValue("Clamd", "ClamdPid", "n/a");
        m_setupFile->setSectionValue("Clamd", "ClamonaccPid", "n/a");
        if (m_dirsUnderMonitoring > 0)
            m_setupFile->setSectionValue("Clamd", "Status2", "not Running");
        m_setupFile->setSectionValue("Clamd", "Status", "not running");

        emit systemStatusChanged();
    }
}

void clamdManager::slot_startClamdOnStartupCheckBoxClicked()
{
    m_setupFile->setSectionValue("Clamd", "StartClamdOnStartup", m_ui.startClamdOnStartupCheckBox->isChecked());
}

void clamdManager::slot_monitoringAddButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Directory to monitor"), QDir::homePath());
    if (path != "")
    {
        if (m_ui.monitoringComboBox->findText(path) == -1)
        {
            m_ui.monitoringComboBox->addItem(path);
            m_setupFile->setSectionValue("Clamonacc", path, "under monitoring");
            m_clamdConf->addSingleLineValue("OnAccessIncludePath", path, "This option specifies a directory (including all files and directories inside it), which should be scanned on access. This option can be used multiple times. Default: disabled");
            m_dirsUnderMonitoring++;
            if (checkClamdRunning() == true)
                restartClamonacc();
        }
        else {
            QMessageBox::warning(this, tr("WARNING"), tr("Path already under monitoring"));
        }
    }
}

void clamdManager::slot_monitoringDelButtonClicked()
{
    QString entry = m_ui.monitoringComboBox->currentText();
    if (entry != "")
    {
        if (QMessageBox::information(this, tr("Remove Folder from monitoring"),
                                     tr("Path: ") + entry + "\n" + tr("Do you want to remove the folder from the monitoring list?"), QMessageBox::Yes,
                                     QMessageBox::No) == QMessageBox::Yes)
        {
            m_setupFile->removeKeyword("Clamonacc", entry);
            m_ui.monitoringComboBox->removeItem(m_ui.monitoringComboBox->currentIndex());
            m_clamdConf->removeSingleLine("OnAccessIncludePath", entry, "This option specifies a directory (including all files and directories inside it), which should be scanned on access. This option can be used multiple times. Default: disabled");
            m_dirsUnderMonitoring--;
            if (checkClamdRunning() == true)
                restartClamonacc();
        }
    }
}

void clamdManager::slot_restartClamonaccProcessFinished()
{
    slot_findclamonaccProcessFinished(0);
}

void clamdManager::slot_restartClamdButtonClicked()
{
    QString clamonaccOptions;
    if (m_setupFile->getSectionValue("Directories", "MoveInfectedFiles").indexOf("checked") == 0)
    {
        QString path = m_setupFile->getSectionValue("Directories", "MoveInfectedFiles");
        path = path.mid(path.indexOf("|") + 1);
        clamonaccOptions = " --move=" + path;
    }

    if (m_setupFile->getSectionValue("Directories", "CopyInfectedFiles").indexOf("checked") == 0)
    {
        QString path = m_setupFile->getSectionValue("Directories", "CopyInfectedFiles");
        path = path.mid(path.indexOf("|") + 1);
        clamonaccOptions = clamonaccOptions + " --copy=" + path;
    }

    if (m_setupFile->getSectionValue("Directories", "ScanReportToFile").indexOf("checked") == 0)
    {
        QString path = m_setupFile->getSectionValue("Directories", "ScanReportToFile");
        path = path.mid(path.indexOf("|") + 1);
        clamonaccOptions = clamonaccOptions + " --log=" + path;
    }

    if (m_setupFile->keywordExists("Directories", "--verbose") == true)
        clamonaccOptions += " --verbose";
    if (m_setupFile->keywordExists("SelectedOptions", "--remove<equal>yes") == true)
        clamonaccOptions += "--remove";
    if (m_setupFile->keywordExists("SelectedOptions", "--allmatch<equal>yes") == true)
        clamonaccOptions += "--allmatch";


    m_clamdRestartInProgress = true;

    m_ui.startStopClamdPushButton->setEnabled(false);
    m_ui.monitoringAddButton->setEnabled(false);
    m_ui.monitoringDelButton->setEnabled(false);
    m_ui.startStopClamdPushButton->setText(tr("  Clamd restarting. Please wait!"));
    m_ui.clamdIconLabel_2->setMovie(new QMovie(":/icons/icons/gifs/spinning_segments_small.gif"));
    m_ui.clamdIconLabel_2->movie()->start();

    QStringList parameters;

    QFile startclamdFile(QDir::homePath() + "/.clamav-gui/startclamd.sh");
    startclamdFile.remove();
    if (startclamdFile.open(QIODevice::Text | QIODevice::ReadWrite))
    {
        QTextStream stream(&startclamdFile);
        if ((m_setupFile->getSectionValue("Clamd","ClamonaccPid") != "n/a") && (m_setupFile->getSectionValue("Clamd","ClamonaccPid") != ""))
        {
            stream << "#!/bin/bash\n"
                   << "kill -9 " + m_setupFile->getSectionValue("Clamd","ClamdPid") + " && kill -9 " + m_setupFile->getSectionValue("Clamd","ClamonaccPid") + " && sleep 20 && " + m_clamdLocation + " -c " + m_setupFile->getSectionValue("Clamd","ClamdConfPath") +
                          " && " + m_clamonaccLocation + " -c " + m_setupFile->getSectionValue("Clamd","ClamdConfPath") + " -l " + m_setupFile->getSectionValue("Clamd","ClamdLogFile")  + " " + clamonaccOptions;
        }
        else {
            stream << "#!/bin/bash\n"
                   << "kill -9 " + m_setupFile->getSectionValue("Clamd","ClamdPid") + " && sleep 20 && " + m_clamdLocation + " -c " + m_setupFile->getSectionValue("Clamd","ClamdConfPath");
        }
        startclamdFile.close();
        startclamdFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadGroup |
                                      QFileDevice::WriteGroup | QFileDevice::ExeGroup);
    }
    parameters << QDir::homePath() + "/.clamav-gui/startclamd.sh";

    if (m_sudoGUI == "")
        m_sudoGUI = m_setupFile->getSectionValue("Settings", "SudoGUI");

    startProcess(m_startClamdProcess,m_sudoGUI, parameters);

    m_setupFile->setSectionValue("Clamd", "Status", "starting up ...");
    if (m_dirsUnderMonitoring > 0)
        m_setupFile->setSectionValue("Clamd", "Status2", "starting up ...");
    m_setupFile->setSectionValue("Clamd", "ClamdPid", "n/a");
    m_setupFile->setSectionValue("Clamd", "ClamonaccPid", "n/a");

    emit systemStatusChanged();
}

void clamdManager::slot_clamdSettingsChanged()
{
    if (m_initprocessrunning == false)
    {
        if (checkClamdRunning() == true)
        {
            m_ui.restartClamdPushButton->setVisible(true);
            m_ui.restartClamdPushButton->setStyleSheet(selectColor("green"));
        }
    }
}

void clamdManager::restartClamonacc()
{
    QStringList parameters;

    QFile restartclamdFile(QDir::homePath() + "/.clamav-gui/restartclamd.sh");
    if (restartclamdFile.exists())
        restartclamdFile.remove();

    if (restartclamdFile.open(QIODevice::Text | QIODevice::ReadWrite))
    {
        QTextStream stream(&restartclamdFile);
        if (m_clamonaccPid != "")
        {
            if (m_dirsUnderMonitoring > 0)
            {
                stream << "#!/bin/bash\n/bin/kill -9 " + m_setupFile->getSectionValue("Clamd","ClamdPid") + " && " + m_clamonaccLocation + " -c " + m_setupFile->getSectionValue("Clamd","ClamdConfPath") +
                              " -l " + m_setupFile->getSectionValue("Clamd","ClamdLogFile");
            }
            else {
                stream << "#!/bin/bash\n/bin/kill -9 " + m_setupFile->getSectionValue("Clamd","ClamonaccPid");
            }
        }
        else {
            if (m_dirsUnderMonitoring > 0)
            {
                stream << "#!/bin/bash\n" + m_clamonaccLocation + " -c " + m_setupFile->getSectionValue("Clamd","ClamdConfPath") + " -l " +
                              m_setupFile->getSectionValue("Clamd","ClamdLogFile");
            }
        }
        restartclamdFile.close();
        restartclamdFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadGroup |
                                        QFileDevice::WriteGroup | QFileDevice::ExeGroup);
    }

    parameters << QDir::homePath() + "/.clamav-gui/restartclamd.sh";

    if (m_sudoGUI == "")
        m_sudoGUI = m_setupFile->getSectionValue("Settings", "SudoGUI");

    startProcess(m_startClamdProcess,m_sudoGUI, parameters);

    m_setupFile->setSectionValue("Clamd", "Status", "shutting down ...");

    emit systemStatusChanged();
}

bool clamdManager::checkClamdRunning()
{
    bool rc = false;

    QString clamdPID = pidof("clamd");

    if (clamdPID != "")
    {
        rc = true;
        m_ui.startStopClamdPushButton->setStyleSheet(selectColor("green"));
        m_ui.startStopClamdPushButton->setText(tr("  Clamd running - Stop clamd"));
        m_ui.startStopClamdPushButton->setIcon(QIcon(":/icons/icons/stopclamd.png"));
        m_setupFile->setSectionValue("Clamd", "ClamdPid", clamdPID);
        if (pidof("clamonacc") != "")
            m_setupFile->setSectionValue("Clamd", "ClamonaccPid", pidof("clamonacc"));

        emit systemStatusChanged();
    }
    else {
        m_ui.startStopClamdPushButton->setStyleSheet(selectColor("red"));
        m_ui.startStopClamdPushButton->setText(tr("  Clamd not running - Start Clamd"));
        m_ui.startStopClamdPushButton->setIcon(QIcon(":/icons/icons/startclamd.png"));
        m_setupFile->setSectionValue("Clamd", "ClamdPid", "n/a");
        m_setupFile->setSectionValue("Clamd", "ClamonaccPid", "n/a");
        m_setupFile->setSectionValue("Clamd", "Status", "not running");

        emit systemStatusChanged();
    }

    return rc;
}

QString clamdManager::selectColor(QString color)
{
    QString rc = "";

    if (m_monochrome == true)
    {
        rc = css_mono;
    }
    else {
        if (color == "mono")
            rc = css_mono;
        if (color == "red")
            rc = css_red;
        if (color == "green")
            rc = css_green;
    }

    return rc;
}

void clamdManager::getClamdConfElements()
{
    QString possibleValues;
    QString keyword;
    QString content;
    QStringList splitter;
    QString valuetype;
    QString skipvalues = "|Example|OnAccessIncludePath|LogFile|PidFile|LocalSocketMode|";
    int poscounter = 0;
    bool skip = false;
    QStringList clamdConfElement;

    if (m_setupFile->getSectionBoolValue("Setup", "DisableLogHighlighter") == false)
    {
        m_logHighlighter = new highlighter(m_ui.clamdLogPlainTextEdit->document());
        m_monochrome = false;
        checkClamdRunning();
    }
    else {
        m_monochrome = true;
        m_ui.startStopClamdPushButton->setStyleSheet(selectColor("mono"));
    }

    QFile file(QDir::homePath() + "/.clamav-gui/clamd.conf.man");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        content = stream.readAll();
        file.close();

        content = content.mid(content.indexOf("Example"));
        content = content.left(content.indexOf("NOTES"));
        content = content.trimmed();

        QStringList lines = content.split("\n");

// Fix for manpage output on Manjero Linux
//--------------------------------------------------------
        content = "";
        QString leadingSpaces = "";
        int count = 0;
        while (lines[1].mid(count,1) == " ")
        {
            count ++;
            leadingSpaces = leadingSpaces + " ";
        }
//--------------------------------------------------------

        content = "";

        foreach(QString line, lines)
        {
            if (line != "")
            {
                // Fix for manpage output on Manjero Linux
                //--------------------------------------------------------
                if (line.indexOf(leadingSpaces) == 0)
                {
                    line = line.trimmed();
                    line = "              " + line;
                }
                //--------------------------------------------------------

                if (line.mid(0,14) != "              ")
                {
                    skip = false;
                    line = line.trimmed();
                    poscounter = 0;
                    splitter = line.split(" ");
                    keyword = splitter[0];
                    if (splitter.length() == 2) valuetype = splitter[1]; else valuetype = "VALUETYPE";
                    if (skipvalues.indexOf("|" + keyword + "|") != -1) skip = true; else content = content + "\n";
                }
                if (skip == false)
                {
                    if (poscounter == 0)
                    {
                        line = line.trimmed();
                        if (valuetype == "VALUETYPE")
                        {
                            content = content + line + " VALUETYPE|";
                        }
                        else {
                            content = content + line + "|";
                        }
                    }
                    else {
                        if (line.left(18) == "                  ")
                        {
                            QString helperstring = line.mid(18);
                            if (helperstring.indexOf(" - ") != -1)
                            {
                                QString newValue = helperstring.left(helperstring.indexOf(" - "));
                                possibleValues == ""?possibleValues = possibleValues + newValue:possibleValues = possibleValues + "," + newValue;
                            }
                        }
                        line = line.trimmed();
                        content.right(1) == "|"?content = content + line:content = content + " " + line;
                        if (line.indexOf("Default:") != -1)
                        {
                            QString defaultvalue = line.mid(line.indexOf("Default:") + 9);
                            if (defaultvalue.indexOf(" ") != -1)
                                defaultvalue = defaultvalue.mid(0,defaultvalue.indexOf(" "));

                            if (valuetype == "VALUETYPE")
                            {
                                if ((defaultvalue.indexOf("yes") != -1) || (defaultvalue.indexOf("no") != -1))
                                {
                                    valuetype = "BOOL";
                                }
                                else {
                                    valuetype = "NUMBER";
                                }
                                content = content.replace("VALUETYPE",valuetype);
                            }
                            if (valuetype == "BOOL")
                            {
                                defaultvalue = defaultvalue.left(3);
                                defaultvalue = defaultvalue.trimmed();
                                content = content + "|yes,no," + defaultvalue;
                            }
                            if ((valuetype == "STRING") || (valuetype == "CATEGORY") || (valuetype == "COMMAND") || (valuetype == "REGEX"))
                            {
                                if (defaultvalue == "LOG_LOCAL6")
                                    possibleValues = "LOG_LOCAL6,LOG_MAIL";
                                possibleValues != ""?content = content + "|" + possibleValues + "," + defaultvalue:content = content + "|" + defaultvalue;
                            }
                            if (valuetype == "NUMBER")
                            {
                                if ((defaultvalue == "disabled") && (keyword.indexOf("Socket") != -1))
                                {
                                    content = content + "|0,4096,0";
                                }
                                else {
                                    if (keyword.indexOf("Port") != -1)
                                    {
                                        content = content + "|0,65536," + defaultvalue;
                                    }
                                    else {
                                        int defaultIntValue = defaultvalue.toInt();
                                        if (defaultIntValue < 0) content = content + "|-1,30,-1";
                                        if (defaultIntValue == 0) content = content + "|0,10,0";
                                        if ((defaultIntValue >= 0) && (defaultIntValue < 1000)) content = content + "|0," + QString::number(defaultIntValue * 3) + "," + defaultvalue;
                                        if ((defaultIntValue >= 1000) && (defaultIntValue < 10000)) content = content + "|0," + QString::number(defaultIntValue * 2) + "," + defaultvalue;
                                        if (defaultIntValue >= 10000) content = content + "|0," + QString::number(defaultIntValue * 10) + "," + defaultvalue;
                                    }
                                }
                            }
                            if (valuetype == "SIZE")
                            {
                                int maxIntValue = 0;
                                bool mega = false;
                                if (defaultvalue.indexOf("M") != -1)
                                {
                                    defaultvalue = defaultvalue.replace("M","");
                                    mega = true;
                                }
                                maxIntValue = defaultvalue.toInt() * 2;
                                if (mega == true)
                                {
                                    content = content + "|0," + QString::number(maxIntValue) + "M," + defaultvalue + "M";
                                }
                                else {
                                    content = content + "|0," + QString::number(maxIntValue) + "," + defaultvalue;
                                }
                            }
                            possibleValues = "";
                        }
                    }
                    poscounter++;
                }
            }
        }
        content = content.trimmed();
    }

    while (content.indexOf("  ") != -1) content = content.replace("  "," ");
    clamdConfElement = content.split("\n");

    // STRING, CATEGORY, COMMAND, REGEX ----> checkbox + lineedit
    // NUMBER, SIZE ----> checkbox + spinbox
    // BOOL ----> checkbox + combobox [yes,no,....] "scanoption"

    QString keywordHelper;
    QString label;
    QString optionValues;
    QString element;
    QString group;

    bool checked = false;
    clamdConfStringOption* stringOption;
    clamdconfspinboxoption* spinboxOption;
    clamdconfcomboboxoption* comboboxOption;
    clamdconfmultioption * comboboxmultioption;
    QString language = setupFileHandler::getSectionValue(QDir::homePath() + "/.clamav-gui/settings.ini","Setup","language");
    if (language == "")
        language = "[en_GB]";

    for (int i = 0; i < clamdConfElement.length()-1; i++)
    {
        element = clamdConfElement.at(i);

        QStringList values = element.split("|");
        values.size() > 0?keywordHelper = values.at(0):keywordHelper = "";
        values.size() > 1?label = values.at(1):label="";
        values.size() > 2?optionValues = values.at(2):optionValues = "";
        QStringList splitter = keywordHelper.split(" ");
        splitter.size() > 0?keyword = splitter.at(0):keyword = "";
        splitter.size() > 1?group = splitter.at(1):group = "";

        if (optionValues == "disabled")
            optionValues = "";

        if ((group == "STRING") || (group == "CATEGORY") || (group == "COMMAND") || (group == "REGEX"))
        {
            if (optionValues.indexOf(",") == -1)
            {
                if ((label.indexOf("multiple times") != -1) || (label.indexOf("mul‐ tiple times") != -1))
                {
                    QStringList tempParams = m_clamdConf->getSingleLineValues(keyword);
                    comboboxmultioption = new clamdconfmultioption(this, keyword, checked, label, tempParams, m_clamdConf,m_setupFile);
                    connect(comboboxmultioption,SIGNAL(settingChanged()),this,SLOT(slot_clamdSettingsChanged()));
                    m_ui.layout1->addWidget(comboboxmultioption);
                    m_clamdConfParameters << comboboxmultioption;
                }
                else {
                    stringOption = new clamdConfStringOption(this, keyword, checked, label, optionValues, m_clamdConf);
                    connect(stringOption, SIGNAL(settingChanged()), this, SLOT(slot_clamdSettingsChanged()));
                    m_ui.layout1->addWidget(stringOption);
                    m_clamdConfParameters << stringOption;
                }
            }
            else {
                comboboxOption = new clamdconfcomboboxoption(this, keyword, checked, label, optionValues, m_clamdConf);
                connect(comboboxOption, SIGNAL(settingChanged()), this, SLOT(slot_clamdSettingsChanged()));
                m_ui.layout1->addWidget(comboboxOption);
                m_clamdConfParameters << comboboxOption;
            }
        }

        if ((group == "NUMBER") || (group == "SIZE"))
        {
            if ((label.indexOf("multiple times") != -1) || (label.indexOf("mul‐ tiple times") != -1))
            {
                QStringList tempParams = m_clamdConf->getSingleLineValues(keyword);
                comboboxmultioption = new clamdconfmultioption(this, keyword, checked, label, tempParams, m_clamdConf,m_setupFile);
                connect(comboboxmultioption,SIGNAL(settingChanged()),this,SLOT(slot_clamdSettingsChanged()));
                m_ui.layout1->addWidget(comboboxmultioption);
                m_clamdConfParameters << comboboxmultioption;
            }
            else {
                spinboxOption = new clamdconfspinboxoption(this, keyword, checked, label, optionValues, m_clamdConf);
                connect(spinboxOption, SIGNAL(settingChanged()), this, SLOT(slot_clamdSettingsChanged()));
                m_ui.layout1->addWidget(spinboxOption);
                m_clamdConfParameters << spinboxOption;
            }
        }

        if (group == "BOOL")
        {
            comboboxOption = new clamdconfcomboboxoption(this, keyword, checked, label, optionValues, m_clamdConf);
            connect(comboboxOption, SIGNAL(settingChanged()), this, SLOT(slot_clamdSettingsChanged()));
            m_ui.layout1->addWidget(comboboxOption);
            m_clamdConfParameters << comboboxOption;
        }
    }
}
