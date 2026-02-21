#include "freshclamsetter.h"
#define css_green "background-color:green;color:yellow"
#define css_red "background-color:red;color:yellow"
#define css_mono "background-color:#404040;color:white"

freshclamsetter::freshclamsetter(QWidget* parent, setupFileHandler* setupFile) : QWidget(parent), m_setupFile(setupFile)
{
    QDir tempDir;

    m_ui.setupUi(this);

    m_startup = true;

    m_lockFreshclamConf = true;
    m_updateLogFileWatcher = NULL;
    m_updateLogHighLighter = NULL;
    m_monochrome = true;
    m_ui.groupBox->setStyleSheet("");
    if (m_setupFile->getSectionBoolValue("Setup", "DisableLogHighlighter") == false) {
        m_monochrome = false;
        m_updateLogHighLighter = new highlighter(m_ui.logPlainText->document());
        m_freshclamLogHighLighter = new highlighter(m_ui.deamonLogText->document());
        m_ui.groupBox->setStyleSheet(css_mono);
        m_ui.updateInfoText->setStyleSheet(css_mono);
    }

    m_sudoGUI = m_setupFile->getSectionValue("Settings", "SudoGUI");

    QFile freshclamConfFile(QDir::homePath() + "/.clamav-gui/freshclam.conf");
    freshclamConfFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    m_freshclamConf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/freshclam.conf", this);

    m_updater = new QProcess(this);
    connect(m_updater, SIGNAL(readyReadStandardError()), this, SLOT(slot_updaterHasOutput()));
    connect(m_updater, SIGNAL(readyReadStandardOutput()), this, SLOT(slot_updaterHasOutput()));
    connect(m_updater, SIGNAL(finished(int)), this, SLOT(slot_updaterFinished(int)));

    m_startDeamonProcess = new QProcess(this);
    connect(m_startDeamonProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this,
            SLOT(slot_startDeamonProcessFinished(int, QProcess::ExitStatus)));

    m_pidFileWatcher = new QFileSystemWatcher(this);
    connect(m_pidFileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(slot_fileSystemWatcherTriggered()));

    m_logFileWatcher = new QFileSystemWatcher(this);
    connect(m_logFileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(slot_logFileWatcherTriggered()));

    m_updateLogFileWatcher = new QFileSystemWatcher(this);
    connect(m_updateLogFileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(slot_updateFileWatcherTriggered()));

    m_ps_process = new QProcess;
    connect(m_ps_process, SIGNAL(finished(int)), this, SLOT(slot_ps_processFinished(int)));

    m_freshclamStartupCounter = 5;
    m_startDelayTimer = new QTimer(this);
    m_startDelayTimer->setSingleShot(true);
    connect(m_startDelayTimer, SIGNAL(timeout()), this, SLOT(slot_startDelayTimerExpired()));

    QFile file(QDir::homePath() + "/.clamav-gui/update.log");
    if (tempDir.exists(QDir::homePath() + "/.clamav-gui/update.log") == true) {
        slot_updateFileWatcherTriggered();
        file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser | QFileDevice::WriteUser |
                            QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ReadOther | QFileDevice::WriteOther);
        m_updateLogFileWatcher->addPath(QDir::homePath() + "/.clamav-gui/update.log");
    }
    else if (file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "";
        file.close();
        slot_updateFileWatcherTriggered();
        m_updateLogFileWatcher->addPath(QDir::homePath() + "/.clamav-gui/update.log");
        file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser | QFileDevice::WriteUser |
                            QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ReadOther | QFileDevice::WriteOther);
    }

    QFile fileFreshclamLog(QDir::homePath() + "/.clamav-gui/freshclam.log");
    if (tempDir.exists(QDir::homePath() + "/.clamav-gui/freshclam.log") == true) {
        fileFreshclamLog.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser | QFileDevice::WriteUser |
                                        QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ReadOther | QFileDevice::WriteOther);
        m_logFileWatcher->addPath(QDir::homePath() + "/.clamav-gui/freshclam.log");
    }
    else if (fileFreshclamLog.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&fileFreshclamLog);
        stream << "";
        fileFreshclamLog.close();
        m_logFileWatcher->addPath(QDir::homePath() + "/.clamav-gui/freshclam.log");
        fileFreshclamLog.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser | QFileDevice::WriteUser |
                                        QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ReadOther | QFileDevice::WriteOther);
    }

    m_processWatcher = new QTimer(this);
    connect(m_processWatcher, SIGNAL(timeout()), this, SLOT(slot_processWatcherExpired()));
    m_processWatcher->start(5000);

    checkDaemonRunning();

    initFreshclamSettings();

    setUpdaterInfo();
}

void freshclamsetter::slot_updateNowButtonClicked()
{
    QStringList parameters;

    m_busyLabel = new progressDialog(this);
    m_busyLabel->setStyleSheet("background-color: rgba(192,192,192, 200);");
    m_busyLabel->setGeometry((this->width() - 300) / 2, (this->height() - 160) / 2, 300, 160);
    m_busyLabel->show();
    m_busyLabel->setText(tr("Update process startet ....."));

    setForm(false);

    if (m_setupFile->getSectionBoolValue("FreshClam", "runasroot") == true) {
        QStringList databaseToUpdate;
        databaseToUpdate << "all" << "main" << "daily" << "bytecode";
        QString whatDB = "";
        QString para;
        if (m_setupFile->getSectionIntValue("FreshClam", "DataBaseToUpdate") > 0)
            whatDB = " --update-db=" + databaseToUpdate[m_setupFile->getSectionIntValue("FreshClam", "DataBaseToUpdate")];
        if ((m_setupFile->getSectionValue("Directories", "LoadSupportedDBFiles") != "") &&
            (m_setupFile->getSectionValue("Directories", "LoadSupportedDBFiles").indexOf("not checked") == -1)) {
            para = m_setupFile->getSectionValue("FreshclamSettings", "FreshclamLocation") + " --show-progress --datadir=" +
                   m_setupFile->getSectionValue("Directories", "LoadSupportedDBFiles")
                       .mid(m_setupFile->getSectionValue("Directories", "LoadSupportedDBFiles").indexOf("|") + 1) +
                   " 2>&1 > " + QDir::homePath() + "/.clamav-gui/update.log" + " --config-file=" + QDir::homePath() + "/.clamav-gui/freshclam.conf" +
                   whatDB;
        }
        else {
            para = m_setupFile->getSectionValue("FreshclamSettings", "FreshclamLocation") + " 2>&1 > " + QDir::homePath() +
                   "/.clamav-gui/update.log" + " --show-progress --config-file=" + QDir::homePath() + "/.clamav-gui/freshclam.conf" + whatDB;
        }
        QFile startfreshclamFile(QDir::homePath() + "/.clamav-gui/startfreshclam.sh");
        startfreshclamFile.remove();
        if (startfreshclamFile.open(QIODevice::Text | QIODevice::ReadWrite)) {
            QTextStream stream(&startfreshclamFile);
            stream << "#!/bin/bash\n" << para;
            startfreshclamFile.close();
            startfreshclamFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadGroup |
                                              QFileDevice::WriteGroup | QFileDevice::ExeGroup);
        }
        parameters << QDir::homePath() + "/.clamav-gui/startfreshclam.sh";
        if (m_sudoGUI == "")
            m_sudoGUI = m_setupFile->getSectionValue("Settings", "SudoGUI");
        m_updater->start(m_sudoGUI, parameters);
    }
    else {
        QStringList databaseToUpdate;
        databaseToUpdate << "all" << "main" << "daily" << "bytecode";
        QString databaseDir = m_setupFile->getSectionValue("Directories", "LoadSupportedDBFiles");
        QString checked = databaseDir.mid(0, databaseDir.indexOf("|"));
        databaseDir = databaseDir.mid(databaseDir.indexOf("|") + 1);
        if ((databaseDir != "") && (checked == "checked"))
            parameters << "--datadir=" + databaseDir;
        if (m_setupFile->getSectionIntValue("FreshClam", "DataBaseToUpdate") > 0)
            parameters << "--update-db=" + databaseToUpdate[m_setupFile->getSectionIntValue("FreshClam", "DataBaseToUpdate")];
        parameters << "-l" << QDir::homePath() + "/.clamav-gui/update.log";
        parameters << "--show-progress";
        parameters << "--config-file" << QDir::homePath() + "/.clamav-gui/freshclam.conf";
        m_updater->start(m_setupFile->getSectionValue("FreshclamSettings", "FreshclamLocation"), parameters);
    }
    m_updateLogFileWatcher->removePath(QDir::homePath() + "/.clamav-gui/update.log");
    m_updateLogFileWatcher->addPath(QDir::homePath() + "/.clamav-gui/update.log");
}

void freshclamsetter::slot_startStopDeamonButtonClicked()
{
    QStringList parameters;
    if (m_pidFile == "") {
        if (m_logFile != "")
            m_logFileWatcher->removePath(m_logFile);
        m_pidFile = m_freshclamConf->getSingleLineValue("PidFile");
        m_logFile = QDir::homePath() + "/.clamav-gui/freshclam.log";
        m_logFileWatcher->addPath(m_logFile);
        if (m_setupFile->getSectionBoolValue("FreshClam", "runasroot") == true) {
            if (m_startup == false) {
                QString para = m_setupFile->getSectionValue("FreshclamSettings", "FreshclamLocation") + " -d -l " + m_logFile +
                               " --config-file=" + QDir::homePath() + "/.clamav-gui/freshclam.conf";
                QFile startfreshclamFile(QDir::homePath() + "/.clamav-gui/startfreshclam.sh");
                startfreshclamFile.remove();
                if (startfreshclamFile.open(QIODevice::Text | QIODevice::ReadWrite)) {
                    QTextStream stream(&startfreshclamFile);
                    stream << "#!/bin/bash\n" << para;
                    startfreshclamFile.close();
                    startfreshclamFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                                                      QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup);
                }
                parameters << QDir::homePath() + "/.clamav-gui/startfreshclam.sh";
                if (m_sudoGUI == "")
                    m_sudoGUI = m_setupFile->getSectionValue("Settings", "SudoGUI");
                m_startDeamonProcess->start(m_sudoGUI, parameters);
            }
            else {
                m_startDelayTimer->start(2500);
            }
        }
        else {
            parameters << "-d";
            parameters << "-l" << m_logFile;
            parameters << "--config-file" << QDir::homePath() + "/.clamav-gui/freshclam.conf";
            m_startDeamonProcess->start(m_setupFile->getSectionValue("FreshclamSettings", "FreshclamLocation"), parameters);
        }
    }
    else {
        QFile tempFile(m_pidFile);
        QString pidString;
        if (tempFile.exists() && tempFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&tempFile);
            pidString = stream.readLine();
        }
        if (pidString.isEmpty()) 
        {
            return;
        }
        if (m_setupFile->getSectionBoolValue("FreshClam", "runasroot") == true) {
            QString para = "/bin/kill -sigterm " + pidString + " && rm " + m_pidFile;
            QFile stopfreshclamFile(QDir::homePath() + "/.clamav-gui/stopfreshclam.sh");
            stopfreshclamFile.remove();
            if (stopfreshclamFile.open(QIODevice::Text | QIODevice::ReadWrite)) {
                QTextStream stream(&stopfreshclamFile);
                stream << "#!/bin/bash\n" << para;
                stopfreshclamFile.close();
                stopfreshclamFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadGroup |
                                                 QFileDevice::WriteGroup | QFileDevice::ExeGroup);
            }
            parameters << QDir::homePath() + "/.clamav-gui/stopfreshclam.sh";
            if (m_sudoGUI == "")
                m_sudoGUI = m_setupFile->getSectionValue("Settings", "SudoGUI");
            QProcess::execute(m_sudoGUI, parameters);
            checkDaemonRunning();
        }
        else {
            parameters << "-sigterm" << pidString;
            QProcess::execute("kill", parameters);
            checkDaemonRunning();
        }
    }
}

void freshclamsetter::checkDaemonRunning()
{
    QStringList ps_parameters;
    ps_parameters << "-s" << "freshclam";
    m_ps_process->start("pidof", ps_parameters);
}

void freshclamsetter::slot_ps_processFinished(int rc)
{
    QFile tempFile;

    if (rc == 0) {
        m_pidFile = m_freshclamConf->getSingleLineValue("PidFile");
        m_logFile = QDir::homePath() + "/.clamav-gui/freshclam.log";
    }
    else {
        m_pidFile = "";
        m_logFile = "";
    }

    if ((m_pidFile != "") && (tempFile.exists(m_pidFile) == true)) {
        m_freshclamStartupCounter = 0;

        emit freshclamStarted();
        m_ui.startStopDeamonButton->setText(tr("Deamon running - stop deamon"));
        m_ui.startStopDeamonButton->setStyleSheet(selectColor("green"));
        m_ui.updateNowButton->setStyleSheet(selectColor("green"));
        /*        if (monochrome == false) ui->frame->setStyleSheet("background-color:#c0c0c0;"); else ui->frame->setStyleSheet("");*/
        m_ui.startStopDeamonButton->setIcon(QIcon(":/icons/icons/Clam.png"));

        m_setupFile->setSectionValue("Freshclam", "Started", true);

        m_pidFileWatcher->addPath(m_pidFile);

        slot_setFreshclamsettingsFrameState(false);

        QFile tempFile(m_pidFile);
        QString pidString;
        if (tempFile.exists() && tempFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&tempFile);
            pidString = stream.readLine();
        }
        if (!pidString.isEmpty()) {
            m_setupFile->setSectionValue("Freshclam", "Pid", pidString);
            emit systemStatusChanged();
        }
    }
    else {
        m_pidFile = "";

        m_ui.startStopDeamonButton->setText(tr("Deamon not running - start deamon"));
        m_ui.startStopDeamonButton->setStyleSheet(selectColor("red"));
        m_ui.updateNowButton->setStyleSheet(selectColor("green"));

        //        if (monochrome == false) ui->frame->setStyleSheet("background-color:#c0c0c0;"); else ui->frame->setStyleSheet("");
        m_ui.startStopDeamonButton->setIcon(QIcon(":/icons/icons/Clam.png"));

        m_setupFile->setSectionValue("Freshclam", "Started", false);
        m_setupFile->setSectionValue("Freshclam", "Pid", "n/a");

        slot_setFreshclamsettingsFrameState(true);

        if ((m_startup == true) && (m_setupFile->getSectionBoolValue("Freshclam", "StartDaemon") == true)) {
            m_freshclamStartupCounter--;
            if (m_freshclamStartupCounter > 0)
                m_startDelayTimer->start(2500);
        }
        else {
            if (m_startup == true)
                emit freshclamStarted();
        }
    }

    if ((m_logFile != "") && (tempFile.exists(m_logFile) == true)) {
        m_logFileWatcher->addPath(m_logFile);
        slot_logFileWatcherTriggered();
    }
    else {
        m_logFile = QDir::homePath() + "/.clamav-gui/freshclam.log";
        if (tempFile.exists(m_logFile) == true) {
            m_logFileWatcher->addPath(m_logFile);
            slot_logFileWatcherTriggered();
        }
        else {
            QFile file(QDir::homePath() + "/.clamav-gui/freshclam.log");
            if (file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text)) {
                QTextStream stream(&file);
                stream << "";
                file.close();
                m_logFileWatcher->addPath(m_logFile);
                slot_logFileWatcherTriggered();
            }
        }
    }
    m_startup = false;
}

void freshclamsetter::slot_disableUpdateButtons()
{
    setForm(false);
}

void freshclamsetter::slot_startDelayTimerExpired()
{
    QStringList parameters;
    if (m_setupFile->getSectionBoolValue("FreshClam", "runasroot") == true) {
        QString para = m_setupFile->getSectionValue("FreshclamSettings", "FreshclamLocation") + " -d -l " + m_logFile +
                       " --config-file=" + QDir::homePath() + "/.clamav-gui/freshclam.conf";
        QFile startfreshclamFile(QDir::homePath() + "/.clamav-gui/startfreshclam.sh");
        startfreshclamFile.remove();
        if (startfreshclamFile.open(QIODevice::Text | QIODevice::ReadWrite)) {
            QTextStream stream(&startfreshclamFile);
            stream << "#!/bin/bash\n" << para;
            startfreshclamFile.close();
            startfreshclamFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadGroup |
                                              QFileDevice::WriteGroup | QFileDevice::ExeGroup);
        }
        parameters << QDir::homePath() + "/.clamav-gui/startfreshclam.sh";
        if (m_sudoGUI == "")
            m_sudoGUI = m_setupFile->getSectionValue("Settings", "SudoGUI");
        m_startDeamonProcess->start(m_sudoGUI, parameters);
    }
    else {
        parameters << "-d";
        parameters << "-l" << m_logFile;
        parameters << "--config-file" << QDir::homePath() + "/.clamav-gui/freshclam.conf";
        m_startDeamonProcess->start(m_setupFile->getSectionValue("FreshclamSettings", "FreshclamLocation"), parameters);
        m_startup = false;
        emit freshclamStarted();
    }
}

void freshclamsetter::slot_updaterFinished(int rc)
{
    delete m_busyLabel;
    QString rcstring = m_updater->readAll();

    if (rc == 0) {
        emit setBallonMessage(0, tr("INFO"), tr("Update-Process finished"));
        setUpdaterInfo();
    }
    else {
        emit setBallonMessage(1, tr("WARNING"), tr("Update-Process failed!\nRead log-messages for possible reason."));
    }
    setForm(true);
}

void freshclamsetter::slot_fileSystemWatcherTriggered()
{
    QDir tempDir;

    if ((m_pidFile != "") && (tempDir.exists(m_pidFile) == true)) {
        m_ui.startStopDeamonButton->setText(tr("Deamon running - stop deamon"));
        m_ui.startStopDeamonButton->setStyleSheet(selectColor("green"));
        m_ui.updateNowButton->setStyleSheet("green");

        //        if (monochrome == false) ui->frame->setStyleSheet("background-color:#c0c0c0;"); else ui->frame->setStyleSheet("");

        m_ui.startStopDeamonButton->setIcon(QIcon(":/icons/icons/Clam.png"));
        m_pidFileWatcher->addPath(m_pidFile);
    }
    else {
        m_pidFile = "";
        m_ui.startStopDeamonButton->setText(tr("Deamon not running - start deamon"));
        m_ui.startStopDeamonButton->setStyleSheet(selectColor("red"));
        m_ui.updateNowButton->setStyleSheet(selectColor("green"));
        //        if (monochrome == false) ui->frame->setStyleSheet("background-color:#c0c0c0;"); else ui->frame->setStyleSheet("");

        m_ui.startStopDeamonButton->setIcon(QIcon(":/icons/icons/freshclam.png"));
        m_setupFile->setSectionValue("Freshclam", "Pid", "n/a");
        emit systemStatusChanged();
    }
}

void freshclamsetter::slot_clearLogButtonClicked()
{
    QFile file(QDir::homePath() + "/.clamav-gui/update.log");

    file.remove();
    
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        stream << "";
        file.close();
    }
    m_ui.logPlainText->setPlainText("");
}

void freshclamsetter::slot_logFileWatcherTriggered()
{
    QFile file(m_logFile);
    QString content;
    QString value;
    int pos;

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        content = stream.readAll();
        file.close();
    }

    pos = content.lastIndexOf("ClamAV update process started at");
    if (pos != -1) {
        value = content.mid(pos + 33, content.indexOf("\n", pos + 33) - (pos + 33));
        m_setupFile->setSectionValue("Updater", "LastUpdate", value);
    }

    pos = content.lastIndexOf("main.cvd updated (");
    if (pos != -1) {
        value = content.mid(pos + 17, content.indexOf("\n", pos + 17) - (pos + 17));
        value.replace("(", "");
        value.replace(")", "");
        m_setupFile->setSectionValue("Updater", "MainVersion", value);
    }

    pos = content.lastIndexOf("main.cvd database is up-to-date (");
    if (pos != -1) {
        value = content.mid(pos + 32, content.indexOf("\n", pos + 32) - (pos + 32));
        value.replace("(", "");
        value.replace(")", "");
        m_setupFile->setSectionValue("Updater", "MainVersion", value);
    }

    pos = content.lastIndexOf("daily.cvd updated (");
    if (pos != -1) {
        value = content.mid(pos + 18, content.indexOf("\n", pos + 18) - (pos + 18));
        value.replace("(", "");
        value.replace(")", "");
        m_setupFile->setSectionValue("Updater", "DailyVersion", value);
    }

    pos = content.lastIndexOf("daily.cld updated (");
    if (pos != -1) {
        value = content.mid(pos + 18, content.indexOf("\n", pos + 18) - (pos + 18));
        value.replace("(", "");
        value.replace(")", "");
        m_setupFile->setSectionValue("Updater", "DailyVersion", value);
    }

    pos = content.lastIndexOf("daily.cld database is up-to-date (");
    if (pos != -1) {
        value = content.mid(pos + 33, content.indexOf("\n", pos + 33) - (pos + 33));
        value.replace("(", "");
        value.replace(")", "");
        m_setupFile->setSectionValue("Updater", "DailyVersion", value);
    }

    pos = content.lastIndexOf("bytecode.cvd database is up-to-date (");
    if (pos != -1) {
        value = content.mid(pos + 36, content.indexOf("\n", pos + 36) - (pos + 36));
        value.replace("(", "");
        value.replace(")", "");
        m_setupFile->setSectionValue("Updater", "BytecodeVersion", value);
    }

    pos = content.lastIndexOf("bytecode.cvd updated (");
    if (pos != -1) {
        value = content.mid(pos + 21, content.indexOf("\n", pos + 21) - (pos + 21));
        value.replace("(", "");
        value.replace(")", "");
        m_setupFile->setSectionValue("Updater", "BytecodeVersion", value);
    }

    pos = content.lastIndexOf("Database updated");
    if (pos != -1) {
        value = content.mid(pos, content.indexOf("\n", pos) - (pos));
        m_setupFile->setSectionValue("Updater", "DatabaseFrom", value);
    }

    setUpdaterInfo();
    emit systemStatusChanged();

    m_ui.deamonLogText->clear();
    QStringList lines = content.split("\n");
    foreach (QString line, lines) {
        m_ui.deamonLogText->insertPlainText(line + "\n");
        m_ui.deamonLogText->ensureCursorVisible();
    }
}

void freshclamsetter::slot_updateFileWatcherTriggered()
{
    QFile file(QDir::homePath() + "/.clamav-gui/update.log");
    QString content;
    QString value;
    int pos;

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        content = stream.readAll();
        file.close();
    }
    pos = content.lastIndexOf("ClamAV update process started at");
    if (pos != -1) {
        value = content.mid(pos + 33, content.indexOf("\n", pos + 33) - (pos + 33));
        m_setupFile->setSectionValue("Updater", "LastUpdate", value);
    }

    pos = content.lastIndexOf("main.cvd updated");
    if (pos != -1) {
        value = content.mid(pos + 17, content.indexOf("\n", pos + 17) - (pos + 17));
        value.replace("(", "");
        value.replace(")", "");
        m_setupFile->setSectionValue("Updater", "MainVersion", value);
    }

    pos = content.lastIndexOf("daily.cvd updated");
    if (pos != -1) {
        value = content.mid(pos + 18, content.indexOf("\n", pos + 18) - (pos + 18));
        value.replace("(", "");
        value.replace(")", "");
        m_setupFile->setSectionValue("Updater", "DailyVersion", value);
    }

    pos = content.lastIndexOf("bytecode.cvd updated");
    if (pos != -1) {
        value = content.mid(pos + 21, content.indexOf("\n", pos + 21) - (pos + 21));
        value.replace("(", "");
        value.replace(")", "");
        m_setupFile->setSectionValue("Updater", "BytecodeVersion", value);
    }

    pos = content.lastIndexOf("Database updated");
    if (pos != -1) {
        value = content.mid(pos, content.indexOf("\n", pos) - (pos));
        m_setupFile->setSectionValue("Updater", "DatabaseFrom", value);
    }

    emit systemStatusChanged();

    m_ui.logPlainText->setPlainText("");
    QStringList lines = content.split("\n");
    foreach (QString line, lines) {
        m_ui.logPlainText->insertPlainText(line + "\n");
        m_ui.logPlainText->ensureCursorVisible();
    }
}

void freshclamsetter::setForm(bool mode)
{
    m_ui.clearLogButton->setEnabled(mode);
    m_ui.deamonClearLogButton->setEnabled(mode);
    m_ui.deamonClearLogButton->setEnabled(mode);
    m_ui.startStopDeamonButton->setEnabled(mode);
    m_ui.updateNowButton->setEnabled(mode);
    m_ui.updateSettingsGroupBox->setEnabled(mode);
}

void freshclamsetter::slot_clearDeamonLogButtonClicked()
{
    QFile file(QDir::homePath() + "/.clamav-gui/freshclam.log");

    m_ui.deamonLogText->setPlainText("");
    m_updateLogFileWatcher->removePath(QDir::homePath() + "/.clamav-gui/freshclam.log");
    file.remove();
   
    if (file.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&file);
        stream << "";
        file.close();
    }
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup |
                        QFileDevice::WriteGroup | QFileDevice::ReadOther | QFileDevice::WriteOther);
    m_updateLogFileWatcher->addPath(QDir::homePath() + "/.clamav-gui/freshclam.log");
}

void freshclamsetter::setUpdaterInfo()
{
    QString htmlCode =
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><meta name=\"qrichtext\" "
        "content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style></head><body style=\" font-family:'Sans Serif'; "
        "font-size:9pt; font-weight:400; font-style:normal;\">\n<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; "
        "margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>";
    htmlCode = htmlCode + "<table><tr><td width='200'>";
    if (m_setupFile->getSectionValue("Updater", "DatabaseFrom") != "") {
        htmlCode = htmlCode + tr("Database origin : </td><td>") + m_setupFile->getSectionValue("Updater", "DatabaseFrom") + "</td></tr><tr><td>";
    }
    if (m_setupFile->getSectionValue("Updater", "LastUpdate") != "") {
        htmlCode = htmlCode + tr("Last Update : </td><td>") + m_setupFile->getSectionValue("Updater", "LastUpdate") + "</td></tr><tr><td>";
    }
    if (m_setupFile->getSectionValue("Updater", "MainVersion") != "") {
        htmlCode = htmlCode + tr("Main File : </td><td>") + m_setupFile->getSectionValue("Updater", "MainVersion") + "</td></tr><tr><td>";
    }
    if (m_setupFile->getSectionValue("Updater", "DailyVersion") != "") {
        htmlCode = htmlCode + tr("Daily File : </td><td>") + m_setupFile->getSectionValue("Updater", "DailyVersion") + "</td></tr><tr><td>";
    }
    if (m_setupFile->getSectionValue("Updater", "BytecodeVersion") != "") {
        htmlCode = htmlCode + tr("ByteCode File : </td><td>") + m_setupFile->getSectionValue("Updater", "BytecodeVersion") + "</td></tr></table>";
    }
    htmlCode = htmlCode + "</body></html>";
    m_ui.updateInfoText->setHtml(htmlCode);
}

QString freshclamsetter::extractPureNumber(QString value)
{
    QString rc = "";
    QString validator = "0.123456789";
    bool isValid = true;
    int index = 0;
    value = value.trimmed();
    QString character;

    while ((index < value.length()) && (isValid == true)) {
        character = value.mid(index, 1);
        if (validator.indexOf(character) != -1)
            rc += character;
        else
            isValid = false;
        index++;
    }

    return rc;
}

QString freshclamsetter::selectColor(QString color)
{
    QString rc = "";

    if (m_monochrome == true) {
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

void freshclamsetter::slot_updaterHasOutput()
{
    static QString oldLine;
    QString output = m_updater->readAll();
    int start = output.lastIndexOf("]") + 1;
    int end = output.lastIndexOf("[");
    QString line = output.mid(start, end - start - 1);
    if (line != "") {
        QStringList values = line.split("/");
        if (values.size() == 2) {
            QString maxValueString = values.at(1);
            QString valueString = values.at(0);
            maxValueString = extractPureNumber(maxValueString);
            valueString = extractPureNumber(valueString);
            double valueMax = maxValueString.toDouble();
            double value = valueString.toDouble();
            m_busyLabel->setProgressBarMaxValue(valueMax);
            m_busyLabel->setProgressBarValue(value);
        }
    }
    line = "";
    if ((output.indexOf("Testing database:") > -1) && (oldLine != "Testing Database"))
        line = "Testing Database";
    if ((output.indexOf("bytecode database available for download") != -1) && (oldLine != "Downloading bytecode.cvd"))
        line = "Downloading bytecode.cvd";
    if ((output.indexOf("main database available for download") != -1) && (oldLine != "Downloading main.cvd"))
        line = "Downloading main.cvd";
    if ((output.indexOf("daily database available for download") != -1) && (oldLine != "Downloading daily.cvd"))
        line = "Downloading daily.cvd";
    if (line != "") {
        m_busyLabel->setText(line);
        oldLine = line;
    }
}

void freshclamsetter::slot_startDeamonProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if ((exitCode != 0) || (exitStatus == QProcess::CrashExit))
        m_freshclamStartupCounter = 0;
    if (exitCode == 0) {
        m_ui.startStopDeamonButton->setText(tr("Deamon running - stop deamon"));
        m_ui.startStopDeamonButton->setStyleSheet(selectColor("green"));
        m_ui.updateNowButton->setStyleSheet(selectColor("green"));
        //        if (monochrome == false) ui->frame->setStyleSheet("background-color:#c0c0c0;"); else ui->frame->setStyleSheet("");

        m_ui.startStopDeamonButton->setIcon(QIcon(":/icons/icons/Clam.png"));
        m_pidFileWatcher->addPath(m_pidFile);
        m_logFileWatcher->addPath(m_logFile);
        slot_logFileWatcherTriggered();
        m_pidFileWatcher->addPath(m_pidFile);
        checkDaemonRunning();
    }
    else {
        m_pidFile = "";
        m_ui.startStopDeamonButton->setText(tr("Deamon not running - start deamon"));
        m_ui.startStopDeamonButton->setStyleSheet(selectColor("red"));
        m_ui.updateNowButton->setStyleSheet(selectColor("green"));
        //        if (monochrome == false) ui->frame->setStyleSheet("background-color:#c0c0c0;"); else ui->frame->setStyleSheet("");

        m_ui.startStopDeamonButton->setIcon(QIcon(":/icons/icons/Clam.png"));
        m_setupFile->setSectionValue("Freshclam", "Pid", "n/a");
        emit systemStatusChanged();
    }
}

void freshclamsetter::initFreshclamSettings()
{
    QStringList parameters;
    QFile tempFile;

    m_lockFreshclamConf = true;
    //m_setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini", this);
    if (m_setupFile->keywordExists("FreshClam", "runasroot") == true)
        m_ui.runasrootCheckBox->setChecked(m_setupFile->getSectionBoolValue("FreshClam", "runasroot"));
    else {
        m_setupFile->setSectionValue("FreshClam", "runasroot", "true");
        m_ui.runasrootCheckBox->setChecked(true);
    }

    if (m_setupFile->keywordExists("Freshclam", "StartDaemon") == true)
        m_ui.autoStartDaemonCheckBox->setChecked(m_setupFile->getSectionBoolValue("Freshclam", "StartDaemon"));
    else {
        m_setupFile->setSectionValue("Freshclam", "StartDaemon", false);
        m_ui.autoStartDaemonCheckBox->setChecked(false);
    }

    m_getDBUserProcess = new QProcess(this);
    connect(m_getDBUserProcess, SIGNAL(finished(int)), this, SLOT(slot_getDBUserProcessFinished()));

    m_freshclamLocationProcess = new QProcess(this);
    connect(m_freshclamLocationProcess, SIGNAL(finished(int)), this, SLOT(slot_freshclamLocationProcessFinished()));
    connect(m_freshclamLocationProcess, SIGNAL(readyRead()), this, SLOT(slot_freshclamLocationProcessHasOutput()));

    m_clamscanLocationProcess = new QProcess(this);
    connect(m_clamscanLocationProcess, SIGNAL(finished(int)), this, SLOT(slot_clamscanLocationProcessFinished()));
    connect(m_clamscanLocationProcess, SIGNAL(readyRead()), this, SLOT(slot_clamscanLocationProcessHasOutput()));

    m_freshclamConf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/freshclam.conf", this);
    if (m_freshclamConf->singleLineExists("DatabaseDirectory") == true) {
        m_ui.databaseDirectoryPathLabel->setText(m_freshclamConf->getSingleLineValue("DatabaseDirectory"));
    }
    else {
        QDir tempdir;
        if ((tempdir.exists("/var/lib/clamav") == true) && ((tempFile.exists("/var/lib/clamav/freshclam.dat") == true))) {
            m_freshclamConf->setSingleLineValue("DatabaseDirectory", "/var/lib/clamav");
            m_ui.runasrootCheckBox->setChecked(true);
            m_setupFile->setSectionValue("FreshClam", "runasroot", true);
        }
        else {
            m_freshclamConf->setSingleLineValue("DatabaseDirectory", QDir::homePath() + "/.clamav-gui/signatures");
            m_ui.runasrootCheckBox->setChecked(false);
            m_setupFile->setSectionValue("FreshClam", "runasroot", false);
        }
        m_ui.databaseDirectoryPathLabel->setText(m_freshclamConf->getSingleLineValue("DatabaseDirectory"));
    }

    if (m_freshclamConf->singleLineExists("LogSyslog") == true)
        m_ui.logSysLogComboBox->setCurrentText(m_freshclamConf->getSingleLineValue("LogSyslog"));
    else {
        m_freshclamConf->setSingleLineValue("LogSyslog", "no");
        m_ui.logSysLogComboBox->setCurrentText(m_freshclamConf->getSingleLineValue("LogSyslog"));
    }

    if (m_freshclamConf->singleLineExists("LogFacility") == true)
        m_ui.logFacilityComboBox->setCurrentText(m_freshclamConf->getSingleLineValue("LogFacility"));
    else {
        m_freshclamConf->setSingleLineValue("LogFacility", "LOG_LOCAL6");
        m_ui.logFacilityComboBox->setCurrentText(m_freshclamConf->getSingleLineValue("LogFacility"));
    }

    if (m_freshclamConf->singleLineExists("LogTime") == true)
        m_ui.logTimeComboBox->setCurrentText(m_freshclamConf->getSingleLineValue("LogTime"));
    else {
        m_freshclamConf->setSingleLineValue("LogTime", "no");
        m_ui.logTimeComboBox->setCurrentText(m_freshclamConf->getSingleLineValue("LogTime"));
    }

    if (m_freshclamConf->singleLineExists("LogRotate") == true)
        m_ui.logRotateComboBox->setCurrentText(m_freshclamConf->getSingleLineValue("LogRotate"));
    else {
        m_freshclamConf->setSingleLineValue("LogRotate", "no");
        m_ui.logRotateComboBox->setCurrentText(m_freshclamConf->getSingleLineValue("LogRotate"));
    }

    if (m_freshclamConf->singleLineExists("PidFile") == true)
        m_ui.pidFilePathLabel->setText(m_freshclamConf->getSingleLineValue("PidFile"));
    else {
        m_freshclamConf->setSingleLineValue("PidFile", "/tmp/freshclam.pid");
        m_ui.pidFilePathLabel->setText(m_freshclamConf->getSingleLineValue("PidFile"));
    }

    if (m_freshclamConf->singleLineExists("DatabaseOwner") == true)
        m_ui.databaseOwnerLineEdit->setText(m_freshclamConf->getSingleLineValue("DatabaseOwner"));
    else {
        m_freshclamConf->setSingleLineValue("DatabaseOwner", "clamav");
        m_ui.databaseOwnerLineEdit->setText(m_freshclamConf->getSingleLineValue("DatabaseOwner"));
    }

    if (m_freshclamConf->singleLineExists("DatabaseMirror") == true)
        m_ui.databaseMirrorLineEdit->setText(m_freshclamConf->getSingleLineValue("DatabaseMirror"));
    else {
        m_freshclamConf->setSingleLineValue("DatabaseMirror", "database.clamav.net");
        m_ui.databaseMirrorLineEdit->setText(m_freshclamConf->getSingleLineValue("DatabaseMirror"));
    }

    if (m_freshclamConf->singleLineExists("LogVerbose") == true)
        m_ui.logVerboseComboBox->setCurrentText(m_freshclamConf->getSingleLineValue("LogVerbose"));
    else {
        m_freshclamConf->setSingleLineValue("LogVerbose", "no");
        m_ui.logVerboseComboBox->setCurrentText(m_freshclamConf->getSingleLineValue("LogVerbose"));
    }

    if (m_freshclamConf->singleLineExists("Checks") == true)
        m_ui.checkPerDaySpinBox->setValue(m_freshclamConf->getSingleLineValue("Checks").toInt());
    else {
        m_freshclamConf->setSingleLineValue("Checks", "12");
        m_ui.checkPerDaySpinBox->setValue(m_freshclamConf->getSingleLineValue("Checks").toInt());
    }

    m_ui.freshclamLocationLineEdit->setText(m_setupFile->getSectionValue("FreshclamSettings", "FreshclamLocation"));
    m_ui.databaseTypeComboBox->setCurrentIndex(m_setupFile->getSectionIntValue("FreshClam", "DataBaseToUpdate"));

    if (m_freshclamConf->singleLineExists("HTTPProxyServer") == true)
        m_ui.httpProxyServerLineEdit->setText(m_freshclamConf->getSingleLineValue("HTTPProxyServer"));
    if (m_freshclamConf->singleLineExists("HTTPProxyPort") == true)
        m_ui.httpProxyPortLineEdit->setText(m_freshclamConf->getSingleLineValue("HTTPProxyPort"));
    if (m_freshclamConf->singleLineExists("HTTPProxyUsername") == true)
        m_ui.httpProxyUsernameLineEdit->setText(m_freshclamConf->getSingleLineValue("HTTPProxyUsername"));
    if (m_freshclamConf->singleLineExists("HTTPProxyPassword") == true)
        m_ui.httpProxyPasswordLineEdit->setText(m_freshclamConf->getSingleLineValue("HTTPProxyPassword"));
    if (m_freshclamConf->singleLineExists("OnUpdateExecute") == true)
        m_ui.onUpdateExecuteLineEdit->setText(m_freshclamConf->getSingleLineValue("OnUpdateExecute"));
    if (m_freshclamConf->singleLineExists("OnErrorExecute") == true)
        m_ui.onErrorExecuteLineEdit->setText(m_freshclamConf->getSingleLineValue("OnErrorExecute"));
    if (m_freshclamConf->singleLineExists("OnOutdatedExecute") == true)
        m_ui.onOutdatedExecuteLineEdit->setText(m_freshclamConf->getSingleLineValue("OnOutdatedExecute"));

    parameters << "-ld" << m_ui.databaseDirectoryPathLabel->text();
    QDir dbDir;

    if (dbDir.exists(m_ui.databaseDirectoryPathLabel->text()) == true) {
        m_getDBUserProcess->start("ls", parameters);
    }

    m_lockFreshclamConf = false;
    m_clamscanlocationProcessOutput = "";
    m_freshclamlocationProcessOutput = "";

    parameters.clear();
    parameters << "clamscan";
    m_clamscanLocationProcess->start("whereis", parameters);

    parameters.clear();
    parameters << "freshclam";
    m_freshclamLocationProcess->start("whereis", parameters);
}

void freshclamsetter::slot_runasrootCheckBoxChanged()
{
    m_setupFile->setSectionValue("FreshClam", "runasroot", m_ui.runasrootCheckBox->isChecked());
}

void freshclamsetter::slot_writeFreshclamSettings()
{
    if (m_lockFreshclamConf == false) {
        m_freshclamConf->setSingleLineValue("DatabaseDirectory", m_ui.databaseDirectoryPathLabel->text());
        m_freshclamConf->setSingleLineValue("LogSyslog", m_ui.logSysLogComboBox->currentText());
        m_freshclamConf->setSingleLineValue("LogFacility", m_ui.logFacilityComboBox->currentText());
        m_freshclamConf->setSingleLineValue("LogRotate", m_ui.logRotateComboBox->currentText());
        m_freshclamConf->setSingleLineValue("PidFile", m_ui.pidFilePathLabel->text());
        m_freshclamConf->setSingleLineValue("DatabaseOwner", m_ui.databaseOwnerLineEdit->text());
        m_freshclamConf->setSingleLineValue("DatabaseMirror", m_ui.databaseMirrorLineEdit->text());
        m_freshclamConf->setSingleLineValue("LogTime", m_ui.logTimeComboBox->currentText());
        m_freshclamConf->setSingleLineValue("LogVerbose", m_ui.logVerboseComboBox->currentText());

        if (m_ui.onErrorExecuteLineEdit->text() != "")
            m_freshclamConf->setSingleLineValue("OnErrorExecute", m_ui.onErrorExecuteLineEdit->text());
        else {
            m_freshclamConf->setSingleLineValue("OnErrorExecute", "obsolete");
            m_freshclamConf->removeSingleLine("OnErrorExecute", "obsolete");
        }

        if (m_ui.onUpdateExecuteLineEdit->text() != "")
            m_freshclamConf->setSingleLineValue("OnUpdateExecute", m_ui.onUpdateExecuteLineEdit->text());
        else {
            m_freshclamConf->setSingleLineValue("OnUpdateExecute", "obsolete");
            m_freshclamConf->removeSingleLine("OnUpdateExecute", "obsolete");
        }

        if (m_ui.onOutdatedExecuteLineEdit->text() != "")
            m_freshclamConf->setSingleLineValue("OnOutdatedExecute", m_ui.onOutdatedExecuteLineEdit->text());
        else {
            m_freshclamConf->setSingleLineValue("OnOutdatedExecute", "obsolete");
            m_freshclamConf->removeSingleLine("OnOutdatedExecute", "obsolete");
        }

        if (m_ui.httpProxyServerLineEdit->text() != "")
            m_freshclamConf->setSingleLineValue("HTTPProxyServer", m_ui.httpProxyServerLineEdit->text());
        else {
            m_freshclamConf->setSingleLineValue("HTTPProxyServer", "obsolete");
            m_freshclamConf->removeSingleLine("HTTPProxyServer", "obsolete");
        }

        if (m_ui.httpProxyPortLineEdit->text() != "")
            m_freshclamConf->setSingleLineValue("HTTPProxyPort", m_ui.httpProxyPortLineEdit->text());
        else {
            m_freshclamConf->setSingleLineValue("HTTPProxyPort", "obsolete");
            m_freshclamConf->removeSingleLine("HTTPProxyPort", "obsolete");
        }

        if (m_ui.httpProxyUsernameLineEdit->text() != "")
            m_freshclamConf->setSingleLineValue("HTTPProxyUsername", m_ui.httpProxyUsernameLineEdit->text());
        else {
            m_freshclamConf->setSingleLineValue("HTTPProxyUsername", "obsolete");
            m_freshclamConf->removeSingleLine("HTTPProxyUsername", "obsolete");
        }

        if (m_ui.httpProxyPasswordLineEdit->text() != "")
            m_freshclamConf->setSingleLineValue("HTTPProxyPassword", m_ui.httpProxyPasswordLineEdit->text());
        else {
            m_freshclamConf->setSingleLineValue("HTTPProxyPassword", "obsolete");
            m_freshclamConf->removeSingleLine("HTTPProxyPassword", "obsolete");
        }

        m_setupFile->setSectionValue("FreshClam", "UpdatesPerDay", m_ui.checkPerDaySpinBox->value());
        m_setupFile->setSectionValue("FreshClam", "DataBaseToUpdate", m_ui.databaseTypeComboBox->currentIndex());
    }
}

void freshclamsetter::slot_dbPathChanged(QString dbPath)
{
    if (dbPath != "") {
        m_freshclamConf->setSingleLineValue("DatabaseDirectory", dbPath);
        m_ui.databaseDirectoryPathLabel->setText(dbPath);

        QStringList parameters;
        parameters << "-ld" << m_ui.databaseDirectoryPathLabel->text();
        QDir dbDir;
        if (dbDir.exists(m_ui.databaseDirectoryPathLabel->text()) == true) {
            m_getDBUserProcess->start("ls", parameters);
        }
        else {
            m_freshclamConf->setSingleLineValue("DatabaseOwner", "clamav");
        }
    }
}

void freshclamsetter::slot_getDBUserProcessFinished()
{
    QString output = m_getDBUserProcess->readAll();
    QStringList values = output.split(" ");
    if (values.size() > 1) {
        m_freshclamConf->setSingleLineValue("DatabaseOwner", values[2]);
        m_ui.databaseOwnerLineEdit->setText(values[2]);
    }
}

void freshclamsetter::slot_pidFileSelectButtonClicked()
{
    QString rc = QFileDialog::getExistingDirectory(this, tr("Select Folder for the PID-File"), "/tmp");
    if (rc != "") {
        m_freshclamConf->setSingleLineValue("PidFile", rc + "/freshclam.pid");
        m_setupFile->setSectionValue("Freshclam", "PidFile", rc + "/freshclam.pid");
        m_ui.pidFilePathLabel->setText(rc + "/freshclam.pid");
    }
}

void freshclamsetter::slot_freshclamLocationProcessFinished()
{
    m_freshclamlocationProcessOutput = m_freshclamlocationProcessOutput + m_freshclamLocationProcess->readAll();
    if (m_freshclamlocationProcessOutput.length() > 13) {
        QStringList values = m_freshclamlocationProcessOutput.split(" ");
        if (values.size() > 1) {
            if (values.length() > 0)
                m_setupFile->setSectionValue("FreshclamSettings", "FreshclamLocation", values[1].replace("\n", ""));
            else
                m_setupFile->setSectionValue("FreshclamSettings", "FreshclamLocation", "not found");
            m_ui.freshclamLocationLineEdit->setText(values[1]);
            emit systemStatusChanged();
            QFile file(m_ui.databaseDirectoryPathLabel->text() + "/freshclam.dat");
            if (file.exists() == false) {
                if (QMessageBox::warning(this, tr("Virus definitions missing!"),
                                         m_ui.databaseDirectoryPathLabel->text() + "\n" +
                                             tr("No virus definitions found in the database folder. Should the virus definitions be downloaded?"),
                                         QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
                    emit updateDatabase();
                }
            }
        }
    }
    else {
        m_setupFile->setSectionValue("FreshclamSettings", "FreshclamLocation", "not found");
        m_ui.freshclamLocationLineEdit->setText("not found");
        QMessageBox::warning(this, "WARNING", "Freshclam is missing. Please install!", QMessageBox::Ok);
        setForm(false);  //emit disableUpdateButtons();
        // emit quitApplication();
    }
}

void freshclamsetter::slot_clamscanLocationProcessFinished()
{
    m_clamscanlocationProcessOutput = m_clamscanlocationProcessOutput + m_clamscanLocationProcess->readAll();
    if (m_clamscanlocationProcessOutput.length() < 13) {
        QMessageBox::warning(this, "ERROR", "Clamav is missing. Please install!", QMessageBox::Ok);
        // emit quitApplication();
    }
}

void freshclamsetter::slot_freshclamLocationProcessHasOutput()
{
    m_freshclamlocationProcessOutput = m_freshclamlocationProcessOutput + m_freshclamLocationProcess->readAll();
}

void freshclamsetter::slot_clamscanLocationProcessHasOutput()
{
    m_clamscanlocationProcessOutput = m_clamscanlocationProcessOutput + m_clamscanLocationProcess->readAll();
}

void freshclamsetter::slot_setFreshclamsettingsFrameState(bool state)
{
    m_ui.freshclamLocationLineEdit->setEnabled(state);
    m_ui.pidFileSelectPushButton->setEnabled(state);
    m_ui.pidFilePathLabel->setEnabled(state);
    m_ui.databaseOwnerLineEdit->setEnabled(state);
    m_ui.databaseMirrorLineEdit->setEnabled(state);
    m_ui.logSysLogComboBox->setEnabled(state);
    m_ui.logFacilityComboBox->setEnabled(state);
    m_ui.logRotateComboBox->setEnabled(state);
    m_ui.logTimeComboBox->setEnabled(state);
    m_ui.logVerboseComboBox->setEnabled(state);
    m_ui.databaseDirectoryPathLabel->setEnabled(state);
    m_ui.databaseTypeComboBox->setEnabled(state);
    m_ui.checkPerDaySpinBox->setEnabled(state);
    m_ui.runasrootCheckBox->setEnabled(state);
    m_ui.autoStartDaemonCheckBox->setEnabled(state);
    m_ui.httpProxyServerLineEdit->setEnabled(state);
    m_ui.httpProxyPortLineEdit->setEnabled(state);
    m_ui.httpProxyUsernameLineEdit->setEnabled(state);
    m_ui.httpProxyPasswordLineEdit->setEnabled(state);
    m_ui.onUpdateExecuteLineEdit->setEnabled(state);
    m_ui.onUpdateExecutePushButton->setEnabled(state);
    m_ui.onErrorExecuteLineEdit->setEnabled(state);
    m_ui.onErrorExecutePushButton->setEnabled(state);
    m_ui.onOutdatedExecuteLineEdit->setEnabled(state);
    m_ui.onOutdatedExecutePushButton->setEnabled(state);
}

void freshclamsetter::slot_autoStartDaemon()
{
    m_setupFile->setSectionValue("Freshclam", "StartDaemon", m_ui.autoStartDaemonCheckBox->isChecked());
}

void freshclamsetter::slot_onUpdateExecuteButtonClicked()
{
    QString rc =
        QFileDialog::getOpenFileName(this, tr("On Update Execute"), tr("Select a programm that will be executed when the database is updated."));
    if (rc != "")
        m_ui.onUpdateExecuteLineEdit->setText(rc);
    slot_writeFreshclamSettings();
}

void freshclamsetter::slot_onErrorExecuteButtonClicked()
{
    QString rc = QFileDialog::getOpenFileName(this, tr("On Error Execute"), tr("Select a programm that will be executed when an error occured."));
    if (rc != "")
        m_ui.onErrorExecuteLineEdit->setText(rc);
    slot_writeFreshclamSettings();
}

void freshclamsetter::slot_onOutdatedExecuteButtonClicked()
{
    QString rc =
        QFileDialog::getOpenFileName(this, tr("On Outdated Execute"), tr("Select a programm that will be executed when the database is outdated."));
    if (rc != "")
        m_ui.onOutdatedExecuteLineEdit->setText(rc);
    slot_writeFreshclamSettings();
}

void freshclamsetter::slot_processWatcherExpired()
{
    QString freshclamPid = m_setupFile->getSectionValue("Freshclam", "Pid");

    QDir checkDir;
    if (freshclamPid != "n/a") {
        if (checkDir.exists("/proc/" + freshclamPid) == false) {
            m_setupFile->setSectionValue("Freshclam", "Pid", "n/a");
            emit systemStatusChanged();
            checkDaemonRunning();
        }
    }
}

void freshclamsetter::slot_add_remove_highlighter(bool state)
{
    if (state == true) {
        if (m_updateLogHighLighter != NULL) {
            delete m_updateLogHighLighter;
            delete m_freshclamLogHighLighter;
            m_updateLogHighLighter = NULL;
            m_freshclamLogHighLighter = NULL;
            m_ui.groupBox->setStyleSheet("");
            m_ui.updateInfoText->setStyleSheet("");
            m_ui.updateNowButton->setStyleSheet(css_mono);
        }
        m_monochrome = true;
    }
    else {
        if (m_updateLogHighLighter == NULL) {
            m_updateLogHighLighter = new highlighter(m_ui.logPlainText->document());
            m_freshclamLogHighLighter = new highlighter(m_ui.deamonLogText->document());
            m_ui.groupBox->setStyleSheet(css_mono);
            m_ui.updateInfoText->setStyleSheet(css_mono);
        }
        else {
            delete m_updateLogHighLighter;
            delete m_freshclamLogHighLighter;
            m_updateLogHighLighter = new highlighter(m_ui.logPlainText->document());
            m_freshclamLogHighLighter = new highlighter(m_ui.deamonLogText->document());
            m_ui.groupBox->setStyleSheet(css_mono);
            m_ui.updateInfoText->setStyleSheet(css_mono);
        }
        m_ui.startStopDeamonButton->setStyleSheet(css_green);
        m_ui.updateNowButton->setStyleSheet(css_green);
        m_monochrome = false;
    }
    checkDaemonRunning();
}
