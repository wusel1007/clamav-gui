#include "profilemanager.h"
#include "ui_profilemanager.h"

ProfileManager::ProfileManager(QWidget* parent, setupFileHandler* setupFile) : QWidget(parent), m_setupFile(setupFile), m_ui(new Ui::ProfileManager)
{
    // m_setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini", this); --> uses the setupFileHandler provided by the clamav_gui class
    m_ui->setupUi(this);
    getProfileList();
    slot_readProfileSettings();
    checkMonochromeSettings();
}

ProfileManager::~ProfileManager()
{
    delete m_ui;
}

void ProfileManager::getProfileList()
{
    QStringList profiles = m_setupFile->getKeywords("Profiles");
    QStringList selectableProfiles;
    setupFileHandler sf(this);

    foreach (QString profile, profiles) {
        sf.setSetupFileName(QDir::homePath() + "/.clamav-gui/profiles/" + profile + ".ini");
        if (sf.getSectionValue(profile, "Directories") != "")
            selectableProfiles << profile;
    }

    m_ui->profileComboBox->clear();
    m_ui->profileComboBox->addItems(profiles);
}

void ProfileManager::checkMonochromeSettings()
{
    if (m_setupFile->getSectionBoolValue("Setup", "DisableLogHighlighter") == true) {
        m_ui->targetsLabel->setStyleSheet("background-color:#404040;color:white;padding:3px;border-radius:3px");
        m_ui->filenameLabel->setStyleSheet("background-color:#404040;color:white;padding:3px;border-radius:3px");
        m_ui->optionsLabel->setStyleSheet("background-color:#404040;color:white;padding:3px;border-radius:3px");
    }
    else {
        m_ui->targetsLabel->setStyleSheet("background-color:#c0c0c0;color:black;padding:3px;border-radius:3px");
        m_ui->filenameLabel->setStyleSheet("background-color:#c0c0c0;color:black;padding:3px;border-radius:3px");
        m_ui->optionsLabel->setStyleSheet("background-color:#c0c0c0;color:black;padding:3px;border-radius:3px");
    }
}

void ProfileManager::slot_readProfileSettings()
{
    QString profileName = m_ui->profileComboBox->currentText();
    if (profileName == "")
        return;

    setupFileHandler* tempSetupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/profiles/" + profileName + ".ini", this);
    QStringList targets;
    QString targetLabel;
    QStringList options;
    QString optionLabel;
    QString logFile = tempSetupFile->getSectionValue("Directories", "ScanReportToFile");

    targets = tempSetupFile->getSectionValue(profileName, "Directories").split('\n');
    options = tempSetupFile->getKeywords("SelectedOptions");

    if ((targets[0] != "") & (targets.count() > 0))
        targetLabel = targetLabel + targets[0];
    for (int i = 1; i < targets.count(); i++) {
        if ((targets[i] != "") & (targetLabel != "")) {
            targetLabel = targetLabel + "\n" + targets[i];
        }
        else {
            if (targets[i] != "")
                targetLabel = targets[i];
        }
    }

    m_ui->targetsListLabel->setText(targetLabel);

    if (options.count() > 0)
        optionLabel = optionLabel + options[0];
    for (int i = 1; i < options.count(); i++) {
        optionLabel = optionLabel + "\n" + options[i];
    }

    if (tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", "DontScanFileNamesMatchingRegExp").indexOf("not checked") == -1)
        optionLabel = optionLabel + "\n" + "--exclude=" +
                      tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", "DontScanFileNamesMatchingRegExp")
                          .mid(tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", "DontScanFileNamesMatchingRegExp").indexOf("|") + 1);
    if (tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", "DontScanDiretoriesMatchingRegExp").indexOf("not checked") == -1)
        optionLabel = optionLabel + "\n" + "--exclude-dir=" +
                      tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", "DontScanDiretoriesMatchingRegExp")
                          .mid(tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", "DontScanDiretoriesMatchingRegExp").indexOf("|") + 1);
    if (tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", "OnlyScanFileNamesMatchingRegExp").indexOf("not checked") == -1)
        optionLabel = optionLabel + "\n" + "--include=" +
                      tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", "OnlyScanFileNamesMatchingRegExp")
                          .mid(tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", "OnlyScanFileNamesMatchingRegExp").indexOf("|") + 1);
    if (tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", "OnlyScanDiretoriesMatchingRegExp").indexOf("not checked") == -1)
        optionLabel = optionLabel + "\n" + "--include-dir=" +
                      tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", "OnlyScanDiretoriesMatchingRegExp")
                          .mid(tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", "OnlyScanDiretoriesMatchingRegExp").indexOf("|") + 1);

    if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude", "EnablePUAOptions") == true) {
        QStringList keywords;
        QStringList switches;
        keywords << "LoadPUAPacked" << "LoadPUAPWTool" << "LoadPUANetTool" << "LoadPUAP2P" << "LoadPUAIRC" << "LoadPUARAT" << "LoadPUANetToolSpy";
        keywords << "LoadPUAServer" << "LoadPUAScript" << "LoadPUAAndr" << "LoadPUAJava" << "LoadPUAOsx" << "LoadPUATool" << "LoadPUAUnix"
                 << "LoadPUAWin";
        switches << "--include-pua=Packed" << "--include-pua=PWTool" << "--include-pua=NetTool" << "--include-pua=P2P" << "--include-pua=IRC"
                 << "--include-pua=RAT";
        switches << "--include-pua=NetToolSpy" << "--include-pua=Server" << "--include-pua=Script" << "--include-pua=Andr" << "--include-pua=Java";
        switches << "--include-pua=Osx" << "--include-pua=Tool" << "--include-pua=Unix" << "--include-pua=Win";
        for (int i = 0; i < keywords.length(); i++) {
            if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude", keywords.at(i)) == true)
                optionLabel = optionLabel + "\n" + switches.at(i);
        }
    }
    // Scan Limitations
    QString value = "";
    value = tempSetupFile->getSectionValue("ScanLimitations", "Files larger than this will be skipped and assumed clean");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--max-filesize=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "The maximum amount of data to scan for each container file");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--max-scansize=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "The maximum number of files to scan for each container file");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--max-files=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Maximum archive recursion level for container file");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--max-recursion=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Maximum directory recursion level");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--max-dir-recursion=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Maximum size file to check for embedded PE");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--max-embeddedpe=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Maximum size of HTML file to normalize");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--max-htmlnormalized=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Maximum size of normalized HTML file to scan");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--max-htmlnotags=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Maximum size of script file to normalize");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--max-scriptnormalize=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Maximum size zip to type reanalyze");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--max-ziptypercg=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Maximum number of partitions in disk image to be scanned");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--max-partitions=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Maximum number of icons in PE file to be scanned");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--max-iconspe=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Bytecode timeout in milliseconds");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--bytecode-timeout=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Collect and print execution statistics");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--statistics " + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Structured SSN Format");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--structured-ssn-format=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Structured SSN Count");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--structured-ssn-count=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Structured CC Count");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--structured-cc-count=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Structured CC Mode");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--structured-cc-mode=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Max Scan-Time");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--max-scantime=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Max recursion to HWP3 parsing function");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--max-rechwp3=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Max calls to PCRE match function");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--pcre-match-limit=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Max recursion calls to the PCRE match function");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--pcre-recmatch-limit=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Max PCRE file size");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + "--pcre-max-filesize=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations", "Database outdated if older than x days");
    if ((value != "") && (value.indexOf("not checked") == -1))
        optionLabel = optionLabel + "\n" + " --fail-if-cvd-older-than=" + value.mid(value.indexOf("|") + 1);

    if (tempSetupFile->getSectionBoolValue(profileName, "Recursion") == true) {
        if (optionLabel != "") {
            optionLabel = optionLabel + "\n" + "-r";
        }
        else {
            optionLabel = "-r";
        }
    }

    optionLabel = optionLabel.replace("<equal>", "=");
    m_ui->optionsListLabel->setText(optionLabel);

    if (logFile.left(logFile.indexOf("|")) == "checked") {
        logFile = logFile.mid(logFile.indexOf("|") + 1);
    }
    else {
        logFile = "";
    }

    m_ui->filenameTextLabel->setText(logFile);

    delete tempSetupFile;
}

void ProfileManager::slot_addProfileButtonClicked()
{

    m_profileWizard = new ProfileWizardDialog(this);
    connect(m_profileWizard, SIGNAL(signal_profileSaved()), this, SLOT(slot_profileSaved()));
    m_profileWizard->setModal(true);
    m_profileWizard->show();
}

void ProfileManager::slot_editProfileButtonClicked()
{
    QString profileName = m_ui->profileComboBox->currentText();

    if (profileName != "") {
        m_profileWizard = new ProfileWizardDialog(this, profileName);
        connect(m_profileWizard, SIGNAL(signal_profileSaved()), this, SLOT(slot_profileSaved()));
        m_profileWizard->setModal(true);
        m_profileWizard->show();
        slot_readProfileSettings();
    }
}

void ProfileManager::slot_eraseProfileButtonClicked()
{
    QString profileName = m_ui->profileComboBox->currentText();
    QStringList scanJobs = m_setupFile->getKeywords("ScanJobs");
    QString line;
    QString logfileName;
    bool found = false;

    if (profileName != "") {
        foreach (QString scanJob, scanJobs) {
            line = m_setupFile->getSectionValue("ScanJobs", scanJob);
            if (line.indexOf("|" + profileName + "|") != -1)
                found = true;
        }
        if (found == false) {
            int rc = QMessageBox::question(this, tr("WARNING"), tr("Do you realy want to remove this (") + profileName + tr(") profile"),
                                           QMessageBox::Yes, QMessageBox::No);
            QFile tempFile(QDir::homePath() + "/.clamav-gui/profiles/" + m_ui->profileComboBox->currentText() + ".ini");
            if (rc == QMessageBox::Yes) {
                setupFileHandler* sf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/profiles/" + m_ui->profileComboBox->currentText() + ".ini", this);
                logfileName = sf->getSectionValue("Directories", "ScanReportToFile")
                                  .mid(sf->getSectionValue("Directories", "ScanReportToFile").indexOf("|") + 1);
                if (logfileName != "") {
                    if (QMessageBox::question(this, tr("Info"),
                                              tr("There is a log-file associated with this profile. Shall I remove the log-file as well?"),
                                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
                        QFile fileRemover(logfileName);
                        fileRemover.remove();
                    }
                }
                m_setupFile->removeKeyword("Profiles", m_ui->profileComboBox->currentText());
                tempFile.remove();
                QMessageBox::information(this, tr("Info"), tr("Profile \"") + m_ui->profileComboBox->currentText() + tr("\" removed"));
                getProfileList();
                slot_readProfileSettings();
                emit triggerProfilesChanged();
                delete sf;
            }
        }
        else {
            QMessageBox::information(this, tr("ERROR"), tr("Profile can not be removed because it is in use by the scheduler!"));
        }
    }
}

void ProfileManager::slot_profileSaved()
{
    emit triggerProfilesChanged();
    getProfileList();
}

void ProfileManager::monochromeModeChanged(bool state)
{
    Q_UNUSED(state)
    checkMonochromeSettings();
}
