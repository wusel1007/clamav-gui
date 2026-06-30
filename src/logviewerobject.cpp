#include "logviewerobject.h"
#include "ui_logviewerobject.h"

logViewerObject::logViewerObject(QWidget* parent, setupFileHandler* setupFile) : QWidget(parent), m_setupfile(setupFile), m_ui(new Ui::logViewerObject)
{
    m_ui->setupUi(this);
    m_currentWatcher = nullptr;
    slot_profilesChanged();
}

logViewerObject::~logViewerObject()
{
    delete m_ui;
}

void logViewerObject::slot_profilesChanged()
{
    QStringList profiles = m_setupfile->getKeywords("Profiles");
    QStringList profilesWithLog;
    QStringList values;
    QString actualProfileName = m_ui->profileComboBox->currentText();
    setupFileHandler sf(this);

    foreach (QString profile, profiles)
    {
        sf.setSetupFileName(QDir::homePath() + "/.clamav-gui/profiles/" + profile + ".ini");
        values = sf.getSectionValue("Directories", "ScanReportToFile").split("|");
        if (values.count() == 2)
        {
            QFile tempFile(values[1]);
            if (tempFile.exists())
                profilesWithLog << profile;
        }
    }

    m_ui->profileComboBox->clear();

    QString SRTFvalue = m_setupfile->getSectionValue("Directories","ScanReportToFile");
    if (SRTFvalue.indexOf("checked|") == 0)
    {
        m_ui->profileComboBox->addItem("Direct Scan");
    }

    m_ui->profileComboBox->addItems(profilesWithLog);
    if (m_ui->profileComboBox->findText(actualProfileName) != -1)
    {
        m_ui->profileComboBox->setCurrentIndex(m_ui->profileComboBox->findText(actualProfileName));
        loadLogFile(actualProfileName);
    }
    else {
        loadLogFile(m_ui->profileComboBox->currentText());
    }
}

void logViewerObject::loadLogFile(QString profile)
{
    setupFileHandler* sf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/profiles/" + profile + ".ini", this);
    bool css = m_setupfile->getSectionBoolValue("Setup", "DisableLogHighlighter");
    QString buffer;
    QStringList logs;
    QString tabHeader;
    QStringList values;

    while (m_ui->logTab->count() > 0)
    {
        QWidget * tempwidget = m_ui->logTab->widget(0);
        m_ui->logTab->removeTab(0);

        if (tempwidget != nullptr)
        {
            delete tempwidget;
        }
    }

    if (profile == "Direct Scan")
    {
        values = m_setupfile->getSectionValue("Directories", "ScanReportToFile").split("|");
    } else
    {
        values = sf->getSectionValue("Directories", "ScanReportToFile").split("|");
    }
    if (values.count() == 2)
    {
        QFile file(values[1]);
        m_logFileName = values[1];
        if (file.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&file);
            buffer = stream.readAll();
            logs = buffer.split("<Scanning startet>");
            for (int i = 1; i < logs.count(); i++)
            {
                partialLogObject* log = new partialLogObject(this, logs[i], css);
                connect(this, SIGNAL(logHighlightingChanged(bool)), log, SLOT(slot_add_remove_highlighter(bool)));
                tabHeader = logs[i].mid(1, logs[i].indexOf("\n") - 1);
                while (tabHeader.right(1) == "-") tabHeader = tabHeader.mid(0,tabHeader.length()-1);
                m_ui->logTab->insertTab(0, log, QIcon(":/icons/icons/information.png"), tabHeader);
            }
            m_ui->logTab->setCurrentIndex(0);
            file.close();
            if (m_currentWatcher != nullptr)
                delete m_currentWatcher;
            m_currentWatcher = new QFileSystemWatcher(this);
            m_currentWatcher->addPath(values[1]);
            connect(m_currentWatcher,SIGNAL(fileChanged(QString)),this,SLOT(slot_profilesChanged()));
        }
    }

    delete sf;
}

void logViewerObject::slot_profileSeclectionChanged()
{
    loadLogFile(m_ui->profileComboBox->currentText());
}

void logViewerObject::saveLog()
{
    QString logText;
    partialLogObject* log;

    QFile logFile(m_logFileName);
    if (logFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        QTextStream stream(&logFile);
        for (int i = 0; i < m_ui->logTab->count(); i++)
        {
            log = (partialLogObject*)m_ui->logTab->widget(i);
            logText = logText + log->getLogText();
        }
        stream << logText;
        logFile.close();
    }
}

void logViewerObject::slot_clearLogButtonClicked()
{
    int currentTab = m_ui->logTab->currentIndex();

    if (currentTab > -1)
    {
        if (QMessageBox::question(this, tr("Clear Log"), tr("Do you realy want to remove this partial log?"), QMessageBox::Yes, QMessageBox::No) ==
            QMessageBox::Yes)
        {
            m_ui->logTab->removeTab(currentTab);
            saveLog();
        }
    }
}

void logViewerObject::slot_clearAllButtonClicked()
{
    int count = m_ui->logTab->count();

    if ((count > 0) && (QMessageBox::question(this, tr("Clear Log"), tr("Do you realy want to remove the complete log?"), QMessageBox::Yes,
                                              QMessageBox::No) == QMessageBox::Yes))
    {
        for (int i = 0; i < count; i++)
        {
            m_ui->logTab->removeTab(m_ui->logTab->currentIndex());
        }
    }
    saveLog();
}

void logViewerObject::slot_add_remove_highlighter(bool state)
{
    emit logHighlightingChanged(state);
}
