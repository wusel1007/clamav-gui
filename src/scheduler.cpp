#include "scheduler.h"
#include "sharedvars.cpp"

scheduler::scheduler(QWidget* parent, setupFileHandler* setupFile)
: QWidget(parent), m_setupFile(setupFile)
{
    QStringList header;

    m_ui.setupUi(this);
    header << tr("ID") << tr("Interval") << tr("Profile") << tr("Last Scan") << tr("Next Scan") << tr("Remove") << tr("Scan Now") << tr("Log-File");
    m_ui.scanJobTableWidget->setHorizontalHeaderLabels(header);
    m_ui.scanJobTableWidget->setColumnWidth(6, 120);
    m_ui.scanJobTableWidget->setColumnWidth(5, 120);
    m_removeButtonGroup = new QButtonGroup(this);
    m_scanNowButtonGroup = new QButtonGroup(this);
    m_logButtonGroup = new QButtonGroup(this);
    connect(m_removeButtonGroup, SIGNAL(idClicked(int)), this, SLOT(slot_removeButtonClicked(int)));
    connect(m_scanNowButtonGroup, SIGNAL(idClicked(int)), this, SLOT(slot_scanButtonClicked(int)));
    connect(m_logButtonGroup, SIGNAL(idClicked(int)), this, SLOT(slot_logButtonClicked(int)));
    slot_updateProfiles();
    updateScheduleList();
    m_checkTimer = new QTimer(this);
    m_checkTimer->setSingleShot(false);
    connect(m_checkTimer, SIGNAL(timeout()), this, SLOT(slot_checkTimerTimeout()));
    m_checkTimer->start(15000);
}

void scheduler::slot_addDailyScanJobButtonClicked()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString scanTime = m_ui.dailyTimeEdit->text();
    if (scanTime.length() < 8)
        scanTime = scanTime + ":00";
    QDateTime nextScanObject = QDateTime(QDate::currentDate(), QTime::fromString(scanTime));
    QString id = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString entry;

    if (nextScanObject < currentDateTime)
        nextScanObject = nextScanObject.addDays(1);
    entry = "daily|" + m_ui.profileComboBox->currentText() + "|" + "never" + "|" + QString::number(nextScanObject.toMSecsSinceEpoch());
    m_setupFile->setSectionValue("ScanJobs", id, entry);
    updateScheduleList();
}

void scheduler::slot_addWeeklyScanJobButtonClicked()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDateTime nextScanObject = currentDateTime;
    QString id = QString::number(QDateTime::currentMSecsSinceEpoch());
    int currentDayofweek = currentDateTime.date().dayOfWeek();
    int dayofweek = m_ui.weeklyDayOfWeekComboBox->currentIndex() + 1;
    QString entry;
    QString scanTime = m_ui.weeklyTimeEdit->text();

    if (scanTime.length() < 8)
        scanTime = scanTime + ":00";

    if (currentDayofweek < dayofweek)
    {
        int diff = dayofweek - currentDayofweek;
        nextScanObject = nextScanObject.addDays(diff);
    }
    else {
        if (currentDayofweek > dayofweek)
        {
            int diff = 7 - (currentDayofweek - dayofweek);
            nextScanObject = nextScanObject.addDays(diff);
        }
        else {
            if (QTime::currentTime() > QTime::fromString(scanTime))
            {
                nextScanObject = nextScanObject = nextScanObject.addDays(7);
            }
        }
    }
    nextScanObject = QDateTime(nextScanObject.date(), QTime::fromString(scanTime));
    entry = "weekly|" + m_ui.profileComboBox->currentText() + "|" + "never" + "|" + QString::number(nextScanObject.toMSecsSinceEpoch());
    m_setupFile->setSectionValue("ScanJobs", id, entry);
    updateScheduleList();
}

void scheduler::slot_addMonthlyScanJobButtonClicked()
{
    QString entry;
    QString id = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString scanTime = m_ui.monthlyTimeEdit->text();
    if (scanTime.length() < 8)
        scanTime = scanTime + ":00";
    QDateTime nextScanObject =
        QDateTime(QDate(QDate::currentDate().year(), QDate::currentDate().month(), m_ui.monthlyDaySpinBox->value()), QTime::fromString(scanTime));

    if (nextScanObject < QDateTime::currentDateTime())
    {
        nextScanObject = nextScanObject.addMonths(1);
    }
    entry = "monthly|" + m_ui.profileComboBox->currentText() + "|" + "never" + "|" + QString::number(nextScanObject.toMSecsSinceEpoch());
    m_setupFile->setSectionValue("ScanJobs", id, entry);
    updateScheduleList();
}

void scheduler::slot_updateProfiles()
{
    QStringList profiles = m_setupFile->getKeywords("Profiles");
    QStringList selectableProfiles;
    setupFileHandler sf(this);

    foreach (QString profile, profiles)
    {
        sf.setSetupFileName(QDir::homePath() + "/.clamav-gui/profiles/" + profile + ".ini");
        if (sf.getSectionValue(profile, "Directories") != "")
            selectableProfiles << profile;
    }

    m_ui.profileComboBox->clear();
    m_ui.profileComboBox->addItems(selectableProfiles);
    if (selectableProfiles.count() == 0)
    {
        m_ui.dailyAddScanJobButton->setEnabled(false);
        m_ui.weeklyAddScanJobButton->setEnabled(false);
        m_ui.monthlyAddScanJobButton->setEnabled(false);
        m_ui.dailyProfileLabel->setText("--------------");
        m_ui.weeklyProfileLabel->setText("--------------");
        m_ui.monthlyProfileLable->setText("--------------");
    }
    else {
        m_ui.dailyAddScanJobButton->setEnabled(true);
        m_ui.weeklyAddScanJobButton->setEnabled(true);
        m_ui.monthlyAddScanJobButton->setEnabled(true);
        m_ui.dailyProfileLabel->setText(m_ui.profileComboBox->currentText());
        m_ui.weeklyProfileLabel->setText(m_ui.profileComboBox->currentText());
        m_ui.monthlyProfileLable->setText(m_ui.profileComboBox->currentText());
        slot_profileSelectionChanged();
    }
}

void scheduler::updateScheduleList()
{
    QStringList jobs = m_setupFile->getKeywords("ScanJobs");
    QStringList jobData;
    QDateTime tempDateTime;
    int width[8] = {130, 80, 160, 180, 180, 130, 130, 130};
    int id = 0;
    int rowCount;

    while (m_ui.scanJobTableWidget->rowCount() > 0)
    {
        QPushButton* removeButton = (QPushButton*)m_ui.scanJobTableWidget->cellWidget(0, 5);
        QPushButton* scanNowButton = (QPushButton*)m_ui.scanJobTableWidget->cellWidget(0, 6);
        QPushButton* logButton = (QPushButton*)m_ui.scanJobTableWidget->cellWidget(0, 7);
        m_removeButtonGroup->removeButton(removeButton);
        m_scanNowButtonGroup->removeButton(scanNowButton);
        m_logButtonGroup->removeButton(logButton);
        m_ui.scanJobTableWidget->removeRow(0);
    }

    foreach (QString job, jobs)
    {
        jobData = m_setupFile->getSectionValue("ScanJobs", job).split("|");
        rowCount = m_ui.scanJobTableWidget->rowCount();
        m_ui.scanJobTableWidget->insertRow(rowCount);
        for (int i = 0; i < 8; i++)
        {
            m_ui.scanJobTableWidget->setColumnWidth(i, width[i]);
        }
        m_ui.scanJobTableWidget->setItem(rowCount, 0, new QTableWidgetItem(job));
        m_ui.scanJobTableWidget->setItem(rowCount, 1, new QTableWidgetItem(jobData[0]));
        m_ui.scanJobTableWidget->setItem(rowCount, 2, new QTableWidgetItem(jobData[1]));
        if (jobData[2] == "never")
        {
            m_ui.scanJobTableWidget->setItem(rowCount, 3, new QTableWidgetItem("Never"));
        }
        else {
            tempDateTime.setMSecsSinceEpoch(jobData[2].toLongLong());
            m_ui.scanJobTableWidget->setItem(rowCount, 3, new QTableWidgetItem(tempDateTime.toString("dd.MM.yyyy 'at' hh:mm")));
        }
        tempDateTime.setMSecsSinceEpoch(jobData[3].toLongLong());
        m_ui.scanJobTableWidget->setItem(rowCount, 4, new QTableWidgetItem(tempDateTime.toString("dd.MM.yyyy 'at' hh:mm")));
        QPushButton* removeButton = new QPushButton(QIcon(":/icons/icons/trash-can.png"), tr("remove task"), this);
        m_removeButtonGroup->addButton(removeButton, id);
        QPushButton* scanNowButton = new QPushButton(QIcon(":/icons/icons/start.png"), tr("scan now"), this);
        m_scanNowButtonGroup->addButton(scanNowButton, id);
        QPushButton* logButton = new QPushButton(QIcon(":/icons/icons/information.png"), tr("Log-File"), this);
        m_logButtonGroup->addButton(logButton, id);
        m_ui.scanJobTableWidget->setCellWidget(rowCount, 5, removeButton);
        m_ui.scanJobTableWidget->setCellWidget(rowCount, 6, scanNowButton);
        m_ui.scanJobTableWidget->setCellWidget(rowCount, 7, logButton);
        for (int i = 0; i < 5; i++)
        {
            m_ui.scanJobTableWidget->item(rowCount, i)->setTextAlignment(Qt::AlignCenter);
        }
        id++;
    }
}

void scheduler::slot_scanButtonClicked(int id)
{
    int rc = QMessageBox::information(this, tr("Start Scan-Job"), tr("Do you realy want to start this Scan-Job?"), QMessageBox::Yes, QMessageBox::No);
    QString profileName;
    qint64 today = QDateTime::currentMSecsSinceEpoch();
    QStringList values;
    QString temp;
    QString scanID;

    if (rc == QMessageBox::Yes)
    {
        QTableWidgetItem* item = (QTableWidgetItem*)m_ui.scanJobTableWidget->item(id, 2);
        QTableWidgetItem* item2 = (QTableWidgetItem*)m_ui.scanJobTableWidget->item(id, 0);
        profileName = item->text();
        scanID = item2->text();
        values = m_setupFile->getSectionValue("ScanJobs", scanID).split("|");
        values[2] = QString::number(today);
        temp = values[0] + "|" + values[1] + "|" + values[2] + "|" + values[3];
        m_setupFile->setSectionValue("ScanJobs", scanID, temp);
        updateScheduleList();
        startScanJob(profileName);
    }
}

void scheduler::slot_removeButtonClicked(int id)
{
    int rc = QMessageBox::information(this, tr("Remove Entry"), tr("Do you realy want to remove this entry?"), QMessageBox::Yes, QMessageBox::No);
    QString jobID;

    if (rc == QMessageBox::Yes)
    {
        QTableWidgetItem* item = (QTableWidgetItem*)m_ui.scanJobTableWidget->item(id, 0);
        jobID = item->text();
        m_setupFile->removeKeyword("ScanJobs", jobID);
        updateScheduleList();
    }
}

void scheduler::slot_logButtonClicked(int id)
{
    QString profileName;
    QString logFile;

    QTableWidgetItem* item = (QTableWidgetItem*)m_ui.scanJobTableWidget->item(id, 2);
    profileName = item->text();
    setupFileHandler* tempSF = new setupFileHandler(QDir::homePath() + "/.clamav-gui/profiles/" + profileName + ".ini", this);
    logFile = tempSF->getSectionValue("Directories", "ScanReportToFile");
    if (logFile.left(logFile.indexOf("|")) == "checked")
    {
        logFile = logFile.mid(logFile.indexOf("|") + 1);
        logViewObject* logViewer = new logViewObject(this, logFile);
        connect(logViewer, SIGNAL(logChanged()), this, SLOT(slot_logChanged()));
        logViewer->setModal(true);
        logViewer->showMaximized();
    }
    else {
        QMessageBox::information(this, tr("INFO"), tr("No active log-file for this profile specified!"));
    }

    delete tempSF;
}

void scheduler::startScanJob(QString profileName)
{
    setupFileHandler* setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/profiles/" + profileName + ".ini", this);
    QStringList parameters;
    QStringList selectedOptions = setupFile->getKeywords("SelectedOptions");
    QStringList directoryOptions = setupFile->getKeywords("Directories");
    QStringList scanLimitations = setupFile->getKeywords("ScanLimitations");
    QString option;
    QString checked;
    QString value;

    if (setupFile->getSectionBoolValue(profileName, "Recursion") == true)
    {
        parameters << "-r";
    }
    for (int i = 0; i < selectedOptions.count(); i++)
    {
        parameters << selectedOptions.at(i).left(selectedOptions.indexOf("|")).replace("<equal>", "=");
    }

    // Directory Options
    for (int i = 0; i < directoryOptions.count(); i++)
    {
        option = directoryOptions.at(i);
        value = setupFile->getSectionValue("Directories", option);
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
                    else {
                        parameters << directoryOptionSwitches.at(idx) + "=" + value;
                    }
                }
            }
        }
    }

    // Scan Limitations
    for (int i = 0; i < scanLimitations.count(); i++)
    {
        option = scanLimitations.at(i);
        value = setupFile->getSectionValue("ScanLimitations", option);
        checked = value.left(value.indexOf("|"));
        value = value.mid(value.indexOf("|") + 1);
        if (checked == "checked")
        {
            for (int idx = 0; idx < scanLimitKeywords.size(); idx ++)
            {
                if (option == scanLimitKeywords.at(idx))
                {
                    parameters << scanLimitSwitches.at(idx) + "=" + value;
                }
            }
        }
    }

    // REGEXP and Include Exclude Options
    for (int idx = 0; idx < inclExclKeywords.size(); idx++)
    {
        if (idx < 4)
        {
            value = setupFile->getSectionValue("REGEXP_and_IncludeExclude",inclExclKeywords.at(idx));
            checked = value.left(value.indexOf("|"));
            value = value.mid(value.indexOf("|") + 1);
            if (checked == "checked") parameters << inclExclSwitches.at(idx) + "=" + value;
        }
        else {
            if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","EnablePUAOptions") == true)
            {
                if ((setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude",inclExclKeywords.at(idx)) == true) &&
                    (inclExclKeywords.at(idx) != "EnablePUAOptions"))
                    parameters << inclExclSwitches.at(idx);
            }
        }
    }

    QStringList directories = setupFile->getSectionValue(profileName, "Directories").split("\n");

    for (int i = 0; i < directories.count(); i++)
    {
        if (directories.at(i) != "")
            parameters << directories.at(i);
    }

    emit triggerScanJob(profileName, parameters);

    delete setupFile;
}

void scheduler::slot_checkTimerTimeout()
{
    QStringList scanJobs = m_setupFile->getKeywords("ScanJobs");
    QStringList values;
    QString line;
    qint64 today = QDateTime::currentMSecsSinceEpoch();
    qint64 scanDate;
    QDateTime scanDateTime;

    foreach (QString scanJob, scanJobs)
    {
        values = m_setupFile->getSectionValue("ScanJobs", scanJob).split("|");
        scanDate = values[3].toLongLong();
        if (scanDate < today)
        {
            if (values[0] == "daily")
            {
                scanDateTime = QDateTime::fromMSecsSinceEpoch(scanDate);
                while (scanDateTime.toMSecsSinceEpoch() < today)
                {
                    scanDateTime = scanDateTime.addDays(1);
                }
            }
            if (values[0] == "weekly")
            {
                scanDateTime = QDateTime::fromMSecsSinceEpoch(scanDate);
                while (scanDateTime.toMSecsSinceEpoch() < today)
                {
                    scanDateTime = scanDateTime.addDays(7);
                }
            }
            if (values[0] == "monthly")
            {
                scanDateTime = QDateTime::fromMSecsSinceEpoch(scanDate);
                while (scanDateTime.toMSecsSinceEpoch() < today)
                {
                    scanDateTime = scanDateTime.addMonths(1);
                }
            }
            line = values[0] + "|" + values[1] + "|" + QString::number(today) + "|" + QString::number(scanDateTime.toMSecsSinceEpoch());
            m_setupFile->setSectionValue("ScanJobs", scanJob, line);
            updateScheduleList();
            startScanJob(values[1]);
        }
    }
}

void scheduler::slot_profileSelectionChanged()
{
    QString profileName = m_ui.profileComboBox->currentText();
    setupFileHandler* tempSetupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/profiles/" + profileName + ".ini", this);
    QStringList targets;
    QString targetLabel;
    QStringList options;
    QString optionLabel;
    QString logFile = tempSetupFile->getSectionValue("Directories", "ScanReportToFile");

    m_ui.dailyProfileLabel->setText(profileName);
    m_ui.weeklyProfileLabel->setText(profileName);
    m_ui.monthlyProfileLable->setText(profileName);

    targets = tempSetupFile->getSectionValue(profileName, "Directories").split('\n');
    options = tempSetupFile->getKeywords("SelectedOptions");

    if ((targets[0] != "") & (targets.count() > 0))
        targetLabel = targetLabel + targets[0];
    for (int i = 1; i < targets.count(); i++)
    {
        if ((targets[i] != "") & (targetLabel != ""))
        {
            targetLabel = targetLabel + "\n" + targets[i];
        }
        else {
            if (targets[i] != "")
                targetLabel = targets[i];
        }
    }

    m_ui.targetInfoLabel->setText(targetLabel);

    if (options.count() > 0)
        optionLabel = optionLabel + options[0];
    for (int i = 1; i < options.count(); i++)
    {
        optionLabel = optionLabel + "\n" + options[i];
    }

    for (int idx = 0; idx < inclExclKeywords.size(); idx++)
    {
        if (idx < 4)
        {
            QString keyword = inclExclKeywords.at(idx);
            QString value = tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", keyword)
                                .mid(tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", keyword).indexOf("|") + 1);
            if ((tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude", keyword).indexOf("not checked") == -1) &&
                    (tempSetupFile->keywordExists("REGEXP_and_IncludeExclude", keyword)))
            {
                optionLabel = optionLabel + "\n" + inclExclSwitches.at(idx) + "=" + value;
            }
        }
        else {
            if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude", "EnablePUAOptions") == true)
            {
                if ((tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude", inclExclKeywords.at(idx)) == true) &&
                    (inclExclKeywords.at(idx) != "EnablePUAOptions"))
                    optionLabel = optionLabel + "\n--include-pua=" + inclExclSwitches.at(idx);
            }
        }
    }

    for (int idx = 0; idx < scanLimitKeywords.size(); idx++)
    {
        QString value = tempSetupFile->getSectionValue("ScanLimitations", scanLimitKeywords.at(idx));
        if ((value != "") && (value.indexOf("not checked") == -1))
            optionLabel = optionLabel + "\n" + scanLimitSwitches.at(idx) + "=" + value.mid(value.indexOf("|") + 1);
    }

    if (tempSetupFile->getSectionBoolValue(profileName, "Recursion") == true)
    {
        if (optionLabel != "")
        {
            optionLabel = optionLabel + "\n" + "-r";
        }
        else {
            optionLabel = "-r";
        }
    }

    optionLabel = optionLabel.replace("<equal>", "=");
    m_ui.optionsInfoLabel->setText(optionLabel);

    if (logFile.left(logFile.indexOf("|")) == "checked")
    {
        logFile = logFile.mid(logFile.indexOf("|") + 1);
    }
    else {
        logFile = "";
    }
    m_ui.logFileLabel->setText("Log-File : " + logFile);

    delete tempSetupFile;
}

void scheduler::slot_logChanged()
{
    emit logChanged();
}

void scheduler::slot_disableScheduler()
{
    m_ui.scanJobTableWidget->setEnabled(false);
}
