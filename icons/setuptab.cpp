#include "setuptab.h"
#define css_red "background-color:red;color:white"
#define css_yellow "background-color:yellow;color:black"
#define css_green "background-color:green;color:yellow"
#define css_mono "background-color:#404040;color:white"

setupTab::setupTab(QWidget* parent, setupFileHandler* setupFile) : QWidget(parent), m_setupFile(setupFile)
{
    m_ui.setupUi(this);

    int index = -1;

    QString langhelper;

    m_supressMessage = true;  // verhindert, dass bei der Initialisierung der Sprachauswahl die Warnmeldung kommt.

    //m_setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini", this); --> uses the setupFileHandler provided by the clamav_gui class
    m_monochrome = false;
    if (m_setupFile->keywordExists("Setup", "DisableLogHighlighter") == true)
        m_monochrome = m_setupFile->getSectionBoolValue("Setup", "DisableLogHighlighter");

    if (m_setupFile->keywordExists("Setup", "language") == true) {
        langhelper = m_setupFile->getSectionValue("Setup", "language");
        index = m_ui.languageSelectComboBox->findText(langhelper, Qt::MatchStartsWith);
        if (index == -1)
            index = m_ui.languageSelectComboBox->findText("[en_GB]", Qt::MatchStartsWith);
        m_ui.languageSelectComboBox->setCurrentIndex(index);
    }
    else {
        QString lang = QLocale::system().name();
        index = m_ui.languageSelectComboBox->findText("[" + lang + "]", Qt::MatchStartsWith);
        if (index == -1)
            index = m_ui.languageSelectComboBox->findText("[en_GB]", Qt::MatchStartsWith);
        m_ui.languageSelectComboBox->setCurrentIndex(index);
    }

    if (m_setupFile->keywordExists("Setup", "WindowState") == true) {
        if (m_setupFile->getSectionValue("Setup", "WindowState") == "minimized")
            m_ui.windowStateComboBox->setCurrentIndex(1);
        else
            m_ui.windowStateComboBox->setCurrentIndex(0);
    }

    if (m_setupFile->keywordExists("Clamd", "ClamdScanMultithreading") == true) {
        m_ui.clamdscanComboBox->setCurrentIndex(m_setupFile->getSectionIntValue("Clamd", "ClamdScanMultithreading"));
    }
    else {
        m_setupFile->setSectionValue("Clamd", "ClamdScanMultithreading", 0);
    }

    if (m_setupFile->keywordExists("Setup", "DisableLogHighlighter") == true) {
        m_ui.logHighlighterCheckBox->setChecked(m_setupFile->getSectionBoolValue("Setup", "DisableLogHighlighter"));
    }
    else {
        m_setupFile->setSectionValue("Setup", "DisableLogHighlighter", false);
    }

    slot_updateSystemInfo();
    m_findTranslationProcess = new QProcess(this);
    connect(m_findTranslationProcess,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slot_findTranslationProcessFinished()));

    QStringList parameters;
    parameters << "/usr/share/clamav-gui";
    m_findTranslationProcess->start("ls",parameters);

    m_supressMessage = false;
}

QString setupTab::checkmonochrome(QString color)
{
    QString rc = "";
    if (m_monochrome == true) {
        rc = css_mono;
    }
    else {
        if (color == "red")
            rc = css_red;
        if (color == "yellow")
            rc = css_yellow;
        if (color == "green")
            rc = css_green;
    }

    return rc;
}

void setupTab::slot_updateSystemInfo()
{
    QString systemInfo;
    if (m_setupFile->keywordExists("Clamd", "ClamdLocation") == true)
        m_ui.clamdPath->setText(m_setupFile->getSectionValue("Clamd", "ClamdLocation").replace("\n", ""));
    if (m_setupFile->keywordExists("Clamd", "ClamonaccLocation") == true)
        m_ui.clamonaccPath->setText(m_setupFile->getSectionValue("Clamd", "ClamonaccLocation").replace("\n", ""));
    if (m_setupFile->keywordExists("FreshclamSettings", "FreshclamLocation") == true)
        m_ui.freshclamPath->setText(m_setupFile->getSectionValue("FreshclamSettings", "FreshclamLocation").replace("\n", ""));

    if (m_setupFile->sectionExists("Updater") == true) {
        m_ui.databasePath->setText(m_setupFile->getSectionValue("Directories", "LoadSupportedDBFiles")
                                       .mid(m_setupFile->getSectionValue("Directories", "LoadSupportedDBFiles").indexOf("|") + 1));
        m_ui.databaseLastUpdate->setText(m_setupFile->getSectionValue("Updater", "LastUpdate"));
        m_ui.databaseMainFile->setText(m_setupFile->getSectionValue("Updater", "MainVersion"));
        m_ui.databaseDailyFile->setText(m_setupFile->getSectionValue("Updater", "DailyVersion"));
        m_ui.databaseBytecodeFile->setText(m_setupFile->getSectionValue("Updater", "BytecodeVersion"));

        QString value = m_setupFile->getSectionValue("Updater", "DailyVersion");
        QString scannerVersion = m_setupFile->getSectionValue("Updater", "Version");
        scannerVersion = scannerVersion.replace("Scanner ", "");
        value = value.mid(value.indexOf(" "), value.indexOf(",") - value.indexOf(" "));
        systemInfo = "<div style='font-size:12px;line-height:20px;'><b>Scanner: <font color='navy'>" + scannerVersion +
                     "</font><br>Database: <font color='navy'>" + value + "</font><br>";
        systemInfo += "Date: <font color='navy'>" + m_setupFile->getSectionValue("Updater", "LastUpdate") + "</font></b></div>";
        emit sendSystemInfo(systemInfo);
    }

    if (m_setupFile->keywordExists("Clamd", "ClamonaccPid") == true) {
        m_ui.clamonaccPID->setText(m_setupFile->getSectionValue("Clamd", "ClamonaccPid"));
        if (m_setupFile->getSectionValue("Clamd", "ClamonaccPid") == "n/a") {
            m_ui.clamonaccActivityLabel->setPixmap(QPixmap(":/icons/icons/gifs/activity.gif"));
            m_ui.clamonaccStatus->setText(m_setupFile->getSectionValue("Clamd", "Status2"));
            m_ui.clamonaccStatus->setStyleSheet(checkmonochrome("red"));
        }
        else {
            m_ui.clamonaccActivityLabel->setMovie(new QMovie(":/icons/icons/gifs/activity.gif"));
            m_ui.clamonaccActivityLabel->movie()->start();
            m_ui.clamonaccStatus->setText("is running");
            m_ui.clamonaccStatus->setStyleSheet(checkmonochrome("green"));
        }
    }

    if (m_setupFile->keywordExists("Clamd", "ClamdPid") == true) {
        m_ui.clamdPID->setText(m_setupFile->getSectionValue("Clamd", "ClamdPid"));
        if (m_setupFile->getSectionValue("Clamd", "ClamdPid") == "n/a") {
            m_ui.clamdActivityLabel->setPixmap(QPixmap(":/icons/icons/gifs/activity.gif"));
            QString message = m_setupFile->getSectionValue("Clamd", "Status");
            if ((message == "starting up ...") || (message == "shutting down ...")) {
                m_ui.clamdStatus->setStyleSheet(checkmonochrome("yellow"));
                m_ui.clamdStatus->setText(message);
                if (m_setupFile->getSectionValue("Clamd", "Status2") != "n/a") {
                    m_ui.clamonaccStatus->setStyleSheet(checkmonochrome("yellow"));
                    m_ui.clamonaccStatus->setText(message);
                }
            }
            if (message == "is running") {
                m_ui.clamdStatus->setStyleSheet(checkmonochrome("green"));
                m_ui.clamdStatus->setText(message);
                if (m_setupFile->getSectionValue("Clamd", "Status2") != "is running") {
                    m_ui.clamonaccStatus->setStyleSheet(checkmonochrome("green"));
                    m_ui.clamonaccStatus->setText(message);
                }
            }
            if ((message == "shut down") || (message == "not running")) {
                m_ui.clamdStatus->setStyleSheet(checkmonochrome("red"));
                m_ui.clamdStatus->setText("is down");
                m_ui.clamonaccStatus->setStyleSheet(checkmonochrome("red"));
                m_ui.clamonaccStatus->setText("is down");
            }
        }
        else {
            m_ui.clamdActivityLabel->setMovie(new QMovie(":/icons/icons/gifs/activity.gif"));
            m_ui.clamdActivityLabel->movie()->start();
            m_ui.clamdStatus->setText("is running");
            m_ui.clamdStatus->setStyleSheet(checkmonochrome("green"));
        }
    }

    if (m_setupFile->keywordExists("Freshclam", "Pid") == true) {
        m_ui.freshclamPID->setText(m_setupFile->getSectionValue("Freshclam", "Pid"));
        if (m_setupFile->getSectionValue("Freshclam", "Pid") == "n/a") {
            m_ui.freshclamActivityLabel->setPixmap(QPixmap(":/icons/icons/gifs/activity.gif"));
            m_ui.freshclamStatus->setText("is down");
            m_ui.freshclamStatus->setStyleSheet(checkmonochrome("red"));
        }
        else {
            m_ui.freshclamActivityLabel->setMovie(new QMovie(":/icons/icons/gifs/activity.gif"));
            m_ui.freshclamActivityLabel->movie()->start();
            m_ui.freshclamStatus->setText("is running");
            m_ui.freshclamStatus->setStyleSheet(checkmonochrome("green"));
        }
    }
}

void setupTab::slot_clamdButtonClicked()
{
    emit switchActiveTab(6);
}

void setupTab::slot_freshclamButtonClicked()
{
    emit switchActiveTab(5);
}

void setupTab::slot_clamdscanComboBoxClicked()
{
    m_setupFile->setSectionValue("Clamd", "ClamdScanMultithreading", m_ui.clamdscanComboBox->currentIndex());
}

void setupTab::slot_logHightlighterCheckBoxClicked()
{
    m_setupFile->setSectionValue("Setup", "DisableLogHighlighter", m_ui.logHighlighterCheckBox->isChecked());
    logHighlightingChanged(m_ui.logHighlighterCheckBox->isChecked());
    m_monochrome = m_ui.logHighlighterCheckBox->isChecked();
    slot_updateSystemInfo();
}

void setupTab::slot_findTranslationProcessFinished()
{
    // load the list with the full names of the languages for the translation files from countryfullnames.txt
    QStringList m_langcode2name;
    QString m_langcodeshelper = "";
    QFile m_file("/usr/share/clamav-gui/languageicons/countryfullnames.txt");
    if (m_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&m_file);
        m_langcodeshelper = stream.readAll().toLocal8Bit().constData();
        m_file.close();
        m_langcode2name = m_langcodeshelper.split("\n");
    }

    QString m_languages = m_findTranslationProcess->readAll();
    QStringList m_languageList = m_languages.split("\n");
    QString m_lang = "";
    QString m_languagefullname = "";
    foreach (m_lang, m_languageList) {
        if (m_lang.indexOf(".qm") != -1) {
            m_lang = m_lang.mid(11,5);
            foreach (QString m_item, m_langcode2name) {
                if (m_item.indexOf(m_lang) == 0) {
                    m_languagefullname = m_item.mid(5);
                }
            }
            m_ui.languageSelectComboBox->addItem(QIcon("/usr/share/clamav-gui/languageicons/" + m_lang + ".png"),"[" + m_lang + "] " + m_languagefullname);
        }
    }
}

void setupTab::slot_clamonaccButtonClicked()
{
    emit switchActiveTab(6);
}

void setupTab::slot_selectedLanguageChanged()
{
    m_setupFile->setSectionValue("Setup", "language", m_ui.languageSelectComboBox->currentText().mid(0, 7));
    if (m_supressMessage == false) {
        QMessageBox::information(this, tr("Warning"), tr("You have to restart the application for changes to take effect!"));
    }
}

void setupTab::slot_basicSettingsChanged()
{
    if (m_ui.windowStateComboBox->currentIndex() == 0)
        m_setupFile->setSectionValue("Setup", "WindowState", "maximized");
    if (m_ui.windowStateComboBox->currentIndex() == 1)
        m_setupFile->setSectionValue("Setup", "WindowState", "minimized");
}
