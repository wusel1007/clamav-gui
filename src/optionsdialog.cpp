#include "optionsdialog.h"
#define css "background-color:#404040;color:white"

optionsDialog::optionsDialog(QWidget* parent, setupFileHandler* setupFile) : QWidget(parent), m_setupFile(setupFile)
{
    m_ui.setupUi(this);
    //m_setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini", this); --> uses the setupFileHandler provided by the clamav_gui class
    updateDirectories();
    m_ui.tabWidget->setTabText(0, tr("Options"));
    m_ui.tabWidget->setTabText(1, tr("Directories"));
    m_scanLimits = new scanLimitsTab(this, m_setupFile);
    m_ui.tabWidget->addTab(m_scanLimits, QIcon(":/icons/icons/scanlimits.png"), tr("Scan Limitations"));
    m_incExcOptTab = new includeExcludeOptions(this,m_setupFile);

    connect(m_incExcOptTab, SIGNAL(updateClamdConf()), this, SLOT(slot_updateClamdConf()));
    connect(m_scanLimits, SIGNAL(updateClamdConf()), this, SLOT(slot_updateClamdConf()));

    m_ui.tabWidget->addTab(m_incExcOptTab, QIcon(":/icons/icons/includeexclude.png"), tr("Include/Exclude"));
    m_ui.tabWidget->setIconSize(QSize(24, 24));

    m_getClamscanParametersProcess = new QProcess(this);
    connect(m_getClamscanParametersProcess, SIGNAL(readyReadStandardError()), this, SLOT(slot_getClamscanProcessHasOutput()));
    connect(m_getClamscanParametersProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(slot_getClamscanProcessHasOutput()));
    connect(m_getClamscanParametersProcess, SIGNAL(finished(int)), this, SLOT(slot_getClamscanProcessFinished()));

    QStringList parameters;
    parameters << "--help";

    m_getClamscanParametersProcess->start("clamscan", parameters);
}

void optionsDialog::slot_updateClamdConf()
{
    emit updateClamdConf();
}

void optionsDialog::slot_getClamscanProcessHasOutput()
{
    m_getClamscanProcessOutput = m_getClamscanProcessOutput + m_getClamscanParametersProcess->readAll();
}


/*
  TODO: refactor this method, too long
 */
void optionsDialog::slot_getClamscanProcessFinished()
{
    QString excludeList = "--help|--version|--database|--log|--file-list|--move|--copy|--exclude|--exclude-dir|--include|--include-dir|";
    excludeList +=
        "--bytecode-timeout|--statistics|--exclude-pua|--include-pua|--structured-ssn-format|--structured-ssn-count|--structured-cc-count|--tempdir|-"
        "-follow-dir-symlinks|";
    excludeList +=
        "--structured-cc-mode|--max-scantime|--max-filesize|--max-scansize|--max-files|--max-recursion|--max-dir-recursion|--max-embeddedpe|--max-"
        "htmlnormalize|--follow-file-symlinks|";
    excludeList +=
        "--max-htmlnotags|--max-scriptnormalize|--max-ziptypercg|--max-partitions|--max-iconspe|--max-rechwp3|--pcre-match-limit|--pcre-recmatch-"
        "limit|--pcre-max-filesize|";
    excludeList += "--fail-if-cvd-older-than=days|--hash-alg|--file-type-hint|--hash-hint|--cvdcertsdir";
    m_getClamscanProcessOutput = m_getClamscanProcessOutput + m_getClamscanParametersProcess->readAll();

    QString value;
    QString line;
    QString keyword;
    QString parameter;
    int commentStart = 0;
    QStringList linehelper = m_getClamscanProcessOutput.split("\n");
    QStringList lines;
    QStringList comments;
    QString comment;

    QString line2 = "";
    bool skip = false;
    for (int x = 0; x < linehelper.size(); x++) {
        if (linehelper[x] == "Environment Variables:") {
            skip = true;
        }
        if (skip == false) {
            if (linehelper[x].indexOf("--") != -1) {
                if (line2 != "") {
                    lines << line2;
                    line2 = "";
                }
                line2 = linehelper[x];
            } else {
                line2 = line2 + " " + linehelper[x];
            }
        }
    }
    if (line2 != "") lines << line2;

    m_setupFile->removeSection("AvailableOptions");
    m_setupFile->removeSection("OtherKeywords");

    QString commentSum = "";
    for (int x = 0; x < lines.size(); x++) {
        line = lines[x];
        line = line.trimmed();
        if (line.indexOf("Clam AntiVirus:") != -1) {
            QString version = line.mid(line.indexOf("Clam AntiVirus:") + 15);
            int endpos = 9;
            while (version.mid(endpos,1) != " ") endpos++;
            version = version.mid(0,endpos);
            if ((version.indexOf("Scanner 1.4.1") != -1) || (version.indexOf("Scanner 1.0.7") != -1)) {
                QFile ca_certFile("/etc/pki/tls/certs/ca-bundle.crt");
                QString message =
                    "WARNING\nThe file \"/etc/pki/tls/certs/ca-bundle.crt\" is missing!\nVersion 1.0.7 and 1.4.1 are known to require this file in "
                    "this specific locaction and without it \"freshclam\" will not work.\nInstall the appropriate package for your distribution.";
                QFile ca_alternative_location("/etc/ssl/ca-bundle.pem");
                if (ca_alternative_location.exists() == true)
                    message =
                        "WARNING\nThe file \"/etc/pki/tls/certs/ca-bundle.crt\" is missing!\nVersion 1.0.7 and 1.4.1 are known to require this file "
                        "in this specific locaction and without it \"freshclam\" will not work.\n\nFound the file in the location "
                        "\"/etc/ssl/ca-bundle.pem\".\n\nDo the following command as root (sudo) to fix this issue:\n\nln -s /etc/ssl/ca-bundle.pem "
                        "/etc/pki/tls/certs/ca-bundle.crt";
                ca_alternative_location.setFileName("/et/ssl/certs/ca-certificates.crt");
                if (ca_alternative_location.exists() == true)
                    message =
                        "WARNING\nThe file \"/etc/pki/tls/certs/ca-bundle.crt\" is missing!\nVersion 1.0.7 and 1.4.1 are known to require this file "
                        "in this specific locaction and without it \"freshclam\" will not work.\n\nFound the file in the location "
                        "\"/etc/ssl/ca-bundle.pem\".\n\nDo the following command as root (sudo) to fix this issue:\n\nln -s "
                        "/etc/ssl/certs/ca-certificates.crt /etc/pki/tls/certs/ca-bundle.crt";
                if (ca_certFile.exists() == false)
                    QMessageBox::warning(this, "WARNING", message);
            }
            m_ui.versionLabel->setText("ClamAV Version:" + version);
            m_setupFile->setSectionValue("Updater", "Version", version);
        }

        if (line.indexOf("--") == 0) {
            commentStart = line.indexOf(" ");
            keyword = line.left(commentStart);
            parameter = "";
            if (keyword.indexOf("[=") != -1) {
                keyword = keyword.left(keyword.indexOf("["));
                int start = line.indexOf("[") + 2;
                int length = line.indexOf("]") - start;
                parameter = line.mid(start, length);
            }
            else {
                keyword = keyword.left(keyword.indexOf("="));
            }
            if (excludeList.indexOf(keyword) == -1) {
                if (value == "")
                    value = keyword;
                else
                    value = value + "\n" + keyword;
                if (parameter != "") {
                    value = value + "=" + parameter;
                }
                comment = line.mid(commentStart);
                comment = comment.trimmed();
                while (comment.indexOf("  ") != -1)
                    comment = comment.replace("  ", " ");
                if (comment.left(1) == "-") {
                    comment = comment.mid(comment.indexOf(" ") + 1);
                }
                comment = tr(comment.toLocal8Bit());
                comments.append(comment);
                commentSum == "" ? commentSum = comment : commentSum = commentSum + "|" + comment;
                if ((parameter == "yes/no(*)") || (parameter == "yes(*)/no")) {
                    if (parameter == "yes/no(*)") {
                        m_setupFile->setSectionValue("AvailableOptions", keyword + "<equal>no", comment);
                        m_setupFile->setSectionValue("AvailableOptions", keyword + "<equal>yes", comment);
                    }
                    else {
                        m_setupFile->setSectionValue("AvailableOptions", keyword + "<equal>yes", comment);
                        m_setupFile->setSectionValue("AvailableOptions", keyword + "<equal>no", comment);
                    }
                }
                else {
                    m_setupFile->setSectionValue("AvailableOptions", keyword, comment);
                }
            }
            else {
                m_setupFile->setSectionValue("OtherKeywords", keyword, "exists");
            }
        }
    }

    /*For Debuggin-Reasons only*/
    /*QFile tempfile("/home/wusel/parameters.txt");
    QStringList commentList = commentSum.split("|");
    if (tempfile.open(QIODevice::WriteOnly|QIODevice::Text)){
        QTextStream stream(&tempfile);
        for (int i = 0; i < commentList.length(); i++) {
            stream << "base = base + \"" << commentList.at(i) << "|\";" << "\n";
        }
    }
    tempfile.close();*/

    QStringList parameters = value.split("\n");
    scanoption* option;
    scanoptionyn* optionyn;
    QString label;
    QString yes_no;
    QString language;
    bool flipflop = false;

    for (int x = 0; x < parameters.length(); x++) {
        label = parameters[x];
        if ((label.indexOf("yes(*)/no") != -1) || (label.indexOf("yes/no(*)") != -1)) {
            label.indexOf("yes/no(*)") == -1 ? yes_no = "yes" : yes_no = "no";
            label = label.left(label.indexOf("="));
            if (m_setupFile->keywordExists("SelectedOptions", label + "<equal>no") == true) {
                label = label + "<equal>no";
            }
            else {
                if (m_setupFile->keywordExists("SelectedOptions", label + "<equal>yes") == true) {
                    label = label + "<equal>yes";
                }
                else {
                    if (yes_no == "yes") {
                        label = label + "<equal>yes";
                    }
                    else {
                        label = label + "<equal>no";
                    }
                }
            }
        }

        language = setupFileHandler::getSectionValue(QDir::homePath() + "/.clamav-gui/settings.ini","Setup","language");
        if (language == "") language = "[en_GB]";
        if (m_setupFile->keywordExists("SelectedOptions", label.replace("=", "<equal>")) == true) {
            if (label.indexOf("<equal>") == -1) {
                option = new scanoption(this, QDir::homePath() + "/.clamav-gui/settings.ini", "SelectedOptions", true, label, comments[x], language);
                connect(option, SIGNAL(valuechanged()), this, SLOT(slot_updateClamdConf()));
                if (flipflop == false) {
                    m_ui.optionLayout->addWidget(option);
                    flipflop = true;
                }
                else {
                    m_ui.optionLayout_2->addWidget(option);
                    flipflop = false;
                }
            }
            else {
                optionyn = new scanoptionyn(this, QDir::homePath() + "/.clamav-gui/settings.ini", "SelectedOptions", true, label, comments[x], language);
                connect(optionyn, SIGNAL(valuechanged()), this, SLOT(slot_updateClamdConf()));
                if (flipflop == false) {
                    m_ui.optionLayout->addWidget(optionyn);
                    flipflop = true;
                }
                else {
                    m_ui.optionLayout_2->addWidget(optionyn);
                    flipflop = false;
                }
            }
        }
        else {
            if (label.indexOf("<equal>") == -1) {
                option = new scanoption(this, QDir::homePath() + "/.clamav-gui/settings.ini", "SelectedOptions", false, label, comments[x], language);
                connect(option, SIGNAL(valuechanged()), this, SLOT(slot_updateClamdConf()));
                if (flipflop == false) {
                    m_ui.optionLayout->addWidget(option);
                    flipflop = true;
                }
                else {
                    m_ui.optionLayout_2->addWidget(option);
                    flipflop = false;
                }
            }
            else {
                optionyn = new scanoptionyn(this, QDir::homePath() + "/.clamav-gui/settings.ini", "SelectedOptions", false, label, comments[x], language);
                connect(optionyn, SIGNAL(valuechanged()), this, SLOT(slot_updateClamdConf()));
                if (flipflop == false) {
                    m_ui.optionLayout->addWidget(optionyn);
                    flipflop = true;
                }
                else {
                    m_ui.optionLayout_2->addWidget(optionyn);
                    flipflop = false;
                }
            }
        }
    }
}

QString optionsDialog::getCopyDirectory()
{
    return m_ui.copyDirectoryLineEdit->text();
}

QString optionsDialog::getMoveDirectory()
{
    return m_ui.moveDirectoryLineEdit->text();
}

void optionsDialog::slot_selectLVDButtonClicked()
{
    QString rc;
    rc = QFileDialog::getExistingDirectory(this, tr("Select Directory"), QDir::homePath() + "/.clamav-gui/signatures", QFileDialog::ShowDirsOnly);
    if (rc.isEmpty()) {
        return;
    }
    m_ui.loadVirusDatabaseLineEdit->setText(rc);
    writeDirectories();
    emit systemStatusChanged();
    if (!m_ui.loadVirusDatabaseCheckBox->isChecked()) {
        return;
    }
    emit databasePathChanged(rc);
    QFile file(rc + "/freshclam.dat");
    if (file.exists()) {
        return;
    }
    if (QMessageBox::warning(this, tr("Virus definitions missing!"),
                                tr("No virus definitions found in the database folder. Should the virus definitions be downloaded?"),
                                QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
        m_setupFile->setSectionValue("Directories","LoadSupportedDBFiles","checked|" + QDir::homePath() + "/.clamav-gui/signatures");
        m_ui.loadVirusDatabaseCheckBox->setChecked(true);
        m_ui.loadVirusDatabaseLineEdit->setText(QDir::homePath() + "/.clamav-gui/signatures");
        emit updateDatabase();
    }
}

void optionsDialog::slot_selectCDButtonClicked()
{
    QString rc;
    rc = QFileDialog::getExistingDirectory(this, tr("Select Directory"), QDir::homePath() + "/.clamav-gui/quarantine");
    if (rc != "")
        m_ui.copyDirectoryLineEdit->setText(rc);
    writeDirectories();
}

void optionsDialog::slot_selectMDButtonClicked()
{
    QString rc;
    rc = QFileDialog::getExistingDirectory(this, tr("Select Directory"), QDir::homePath() + "/.clamav-gui/quarantine");
    if (rc != "")
        m_ui.moveDirectoryLineEdit->setText(rc);

    writeDirectories();
}

void optionsDialog::slot_selectSRTFButtonClicked()
{
    QString rc;
    rc = QFileDialog::getSaveFileName(this, tr("Select File"), QDir::homePath() + "/.clamav-gui/logs");
    if (rc != "")
        m_ui.scanReportToFileLineEdit->setText(rc);
    writeDirectories();
    emit srtfSettingsChanged();
}

void optionsDialog::slot_selectSFFButtonClicked()
{
    QString rc;
    rc = QFileDialog::getOpenFileName(this, tr("Select File"), QDir::homePath());
    if (rc != "")
        m_ui.scanFilesFromFileLineEdit->setText(rc);
    writeDirectories();
}

void optionsDialog::slot_selectTDButtonClicked()
{
    QString rc;
    rc = QFileDialog::getExistingDirectory(this, tr("Select Directory"), QDir::homePath());
    if (rc != "")
        m_ui.tempFilesLineEdit->setText(rc);
    writeDirectories();
}

void optionsDialog::updateDirectories()
{
    QString value;
    QString checked;
    QStringList keywords;

    keywords << "LoadSupportedDBFiles" << "ScanReportToFile" << "ScanFilesFromFile" << "TmpFile" << "MoveInfectedFiles";
    keywords << "CopyInfectedFiles" << "FollowDirectorySymLinks" << "FollowFileSymLinks";

    for (int i = 0; i < keywords.length(); i++) {
        value = m_setupFile->getSectionValue("Directories", keywords.at(i));
        checked = value.left(value.indexOf("|"));
        value = value.mid(value.indexOf("|") + 1);

        if (checked == "checked") {
            switch (i) {
                case 0:
                    m_ui.loadVirusDatabaseCheckBox->setChecked(true);
                    m_ui.databaseFrame->setStyleSheet(css);
                    m_ui.loadVirusDatabaseLineEdit->setEnabled(true);
                    m_ui.selectLVDButton->setEnabled(true);
                    m_ui.loadVirusDatabaseLineEdit->setText(value);
                    break;
                case 1:
                    m_ui.scanReportToFileCheckBox->setChecked(true);
                    m_ui.scanReportFrame->setStyleSheet(css);
                    m_ui.scanReportToFileLineEdit->setEnabled(true);
                    m_ui.selectSCRFButton->setEnabled(true);
                    m_ui.scanReportToFileLineEdit->setText(value);
                    break;
                case 2:
                    m_ui.scanFilesFromFileCheckBox->setChecked(true);
                    m_ui.scanFileFrame->setStyleSheet(css);
                    m_ui.scanFilesFromFileLineEdit->setEnabled(true);
                    m_ui.selectSFFButton->setEnabled(true);
                    m_ui.scanFilesFromFileLineEdit->setText(value);
                    break;
                case 3:
                    m_ui.tempFileCheckBox->setChecked(true);
                    m_ui.tempFileFrame->setStyleSheet(css);
                    m_ui.tempFilesLineEdit->setEnabled(true);
                    m_ui.selectTFButton->setEnabled(true);
                    m_ui.tempFilesLineEdit->setText(value);
                    break;
                case 4:
                    m_ui.moveDirectoryCheckBox->setChecked(true);
                    m_ui.moveFrame->setStyleSheet(css);
                    m_ui.moveDirectoryLineEdit->setEnabled(true);
                    m_ui.selectMDButton->setEnabled(true);
                    m_ui.moveDirectoryLineEdit->setText(value);
                    break;
                case 5:
                    m_ui.copyDirectoryCheckBox->setChecked(true);
                    m_ui.copyFrame->setStyleSheet(css);
                    m_ui.copyDirectoryLineEdit->setEnabled(true);
                    m_ui.selectCFButton->setEnabled(true);
                    m_ui.copyDirectoryLineEdit->setText(value);
                    break;
                case 6:
                    m_ui.followDirectorySymlinksCheckBox->setChecked(true);
                    m_ui.followDirLinksFrame->setStyleSheet(css);
                    m_ui.followDirectorySymlinksComboBox->setEnabled(true);
                    m_ui.followDirectorySymlinksComboBox->setCurrentIndex(value.toInt());
                    break;
                case 7:
                    m_ui.followFileSymlinksCheckBox->setChecked(true);
                    m_ui.followFileLinksFrame->setStyleSheet(css);
                    m_ui.followFileSymlinksComboBox->setEnabled(true);
                    m_ui.followFileSymlinksComboBox->setCurrentIndex(value.toInt());
                    break;
            }
        }
        else {
            switch (i) {
                case 0:
                    m_ui.loadVirusDatabaseCheckBox->setChecked(false);
                    m_ui.databaseFrame->setStyleSheet("");
                    m_ui.loadVirusDatabaseLineEdit->setEnabled(false);
                    m_ui.selectLVDButton->setEnabled(false);
                    m_ui.loadVirusDatabaseLineEdit->setText(value);
                    break;
                case 1:
                    m_ui.scanReportToFileCheckBox->setChecked(false);
                    m_ui.scanReportFrame->setStyleSheet("");
                    m_ui.scanReportToFileLineEdit->setEnabled(false);
                    m_ui.selectSCRFButton->setEnabled(false);
                    m_ui.scanReportToFileLineEdit->setText(value);
                    break;
                case 2:
                    m_ui.scanFilesFromFileCheckBox->setChecked(false);
                    m_ui.scanFileFrame->setStyleSheet("");
                    m_ui.scanFilesFromFileLineEdit->setEnabled(false);
                    m_ui.selectSFFButton->setEnabled(false);
                    m_ui.scanFilesFromFileLineEdit->setText(value);
                    break;
                case 3:
                    m_ui.tempFileCheckBox->setChecked(false);
                    m_ui.tempFileFrame->setStyleSheet("");
                    m_ui.tempFilesLineEdit->setEnabled(false);
                    m_ui.selectTFButton->setEnabled(false);
                    m_ui.tempFilesLineEdit->setText(value);
                    break;
                case 4:
                    m_ui.moveDirectoryCheckBox->setChecked(false);
                    m_ui.moveFrame->setStyleSheet("");
                    m_ui.moveDirectoryLineEdit->setEnabled(false);
                    m_ui.selectMDButton->setEnabled(false);
                    m_ui.moveDirectoryLineEdit->setText(value);
                    break;
                case 5:
                    m_ui.copyDirectoryCheckBox->setChecked(false);
                    m_ui.copyFrame->setStyleSheet("");
                    m_ui.copyDirectoryLineEdit->setEnabled(false);
                    m_ui.selectCFButton->setEnabled(false);
                    m_ui.copyDirectoryLineEdit->setText(value);
                    break;
                case 6:
                    m_ui.followDirectorySymlinksCheckBox->setChecked(false);
                    m_ui.followDirLinksFrame->setStyleSheet("");
                    m_ui.followDirectorySymlinksComboBox->setEnabled(false);
                    m_ui.followDirectorySymlinksComboBox->setCurrentIndex(value.toInt());
                    break;
                case 7:
                    m_ui.followFileSymlinksCheckBox->setChecked(false);
                    m_ui.followFileLinksFrame->setStyleSheet("");
                    m_ui.followFileSymlinksComboBox->setEnabled(false);
                    m_ui.followFileSymlinksComboBox->setCurrentIndex(value.toInt());
                    break;
            }
        }
    }
}

void optionsDialog::writeDirectories()
{
    QString keyword;
    QString value;
    QString checked;

    keyword = "LoadSupportedDBFiles";
    value = m_ui.loadVirusDatabaseLineEdit->text();
    m_ui.loadVirusDatabaseCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    m_setupFile->setSectionValue("Directories", keyword, checked + "|" + value);

    keyword = "ScanReportToFile";
    value = m_ui.scanReportToFileLineEdit->text();
    m_ui.scanReportToFileCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    m_setupFile->setSectionValue("Directories", keyword, checked + "|" + value);

    keyword = "ScanFilesFromFile";
    value = m_ui.scanFilesFromFileLineEdit->text();
    m_ui.scanFilesFromFileCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    m_setupFile->setSectionValue("Directories", keyword, checked + "|" + value);

    keyword = "TmpFile";
    value = m_ui.tempFilesLineEdit->text();
    m_ui.tempFileCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    m_setupFile->setSectionValue("Directories", keyword, checked + "|" + value);

    keyword = "MoveInfectedFiles";
    value = m_ui.moveDirectoryLineEdit->text();
    m_ui.moveDirectoryCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    m_setupFile->setSectionValue("Directories", keyword, checked + "|" + value);

    keyword = "CopyInfectedFiles";
    value = m_ui.copyDirectoryLineEdit->text();
    m_ui.copyDirectoryCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    m_setupFile->setSectionValue("Directories", keyword, checked + "|" + value);

    keyword = "FollowDirectorySymLinks";
    value = QString::number(m_ui.followDirectorySymlinksComboBox->currentIndex());
    m_ui.followDirectorySymlinksCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    m_setupFile->setSectionValue("Directories", keyword, checked + "|" + value);

    keyword = "FollowFileSymLinks";
    value = QString::number(m_ui.followFileSymlinksComboBox->currentIndex());
    m_ui.followFileSymlinksCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    m_setupFile->setSectionValue("Directories", keyword, checked + "|" + value);

    if (m_ui.loadVirusDatabaseCheckBox->isChecked() == true) {
        if (m_ui.loadVirusDatabaseLineEdit->text() != "") {
            emit databasePathChanged(m_ui.loadVirusDatabaseLineEdit->text());
            QFile file(m_ui.loadVirusDatabaseLineEdit->text() + "/main.cvd");
            if (file.exists() == false) {
                if (QMessageBox::warning(this, tr("Database files missing!"),
                                         tr("The virus definition files are missing in the database directory. Start download of the missing files?"),
                                         QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
                    emit updateDatabase();
                }
            }
        }
    }
    else {
        emit databasePathChanged("/var/lib/clamav");
    }
    updateDirectories();
}

void optionsDialog::slot_updateDirectories()
{
    updateDirectories();
}

void optionsDialog::slot_scanReportToFileSettingsChanged()
{
    emit srtfSettingsChanged();
}

void optionsDialog::slot_logViewerButtonClicked()
{
    QFile file;

    if ((m_ui.scanReportToFileCheckBox->isChecked() == true) & (m_ui.scanReportToFileLineEdit->text() != "")) {
        file.setFileName(m_ui.scanReportToFileLineEdit->text());
        if (file.exists()) {
            logViewObject* logView = new logViewObject(this, m_ui.scanReportToFileLineEdit->text());
            logView->setModal(true);
            logView->showMaximized();
        }
    }
}
