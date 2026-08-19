#include "scantab.h"

scanTab::scanTab(QWidget* parent, setupFileHandler* setupFile) : QWidget(parent), m_setupFile(setupFile)
{
    m_ui.setupUi(this);
    m_ui.stopScanButton->setVisible(false);
    //m_setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini", this); --> uses the setupFileHandler provided by the clamav_gui class
    m_logHighLighter = NULL;
    m_monochrome = m_setupFile->getSectionBoolValue("Setup", "DisableLogHighlighter");
    if (m_monochrome == false)
        m_logHighLighter = new highlighter(m_ui.logPlainTextEdit->document());
    m_devicelabel = m_ui.deviceLabel;
    checkMonochromeSettings();

    m_model = new CFileSystemModel;
    m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

    m_ui.treeView->setModel(m_model);
    m_ui.treeView->hideColumn(1);
    m_ui.treeView->hideColumn(2);
    m_ui.treeView->hideColumn(3);

    QStringList directories = m_setupFile->getSectionValue("Settings", "Directories").split("\n");
    m_ui.selectedDirectoriesLabel->setText(m_setupFile->getSectionValue("Settings", "Directories"));
    m_ui.treeView->collapseAll();
    m_model->unCheckAll();
    m_model->setRootPath("/");
    foreach (QString dir, directories)
    {
        if (dir != "")
        {
            m_model->setChecked(dir, true);
            QModelIndex index = m_model->index(dir);
            m_ui.treeView->scrollTo(index);
            m_ui.treeView->setCurrentIndex(index);
        }
    }

    m_ui.recursivCheckBox->setChecked(m_setupFile->getSectionBoolValue("Settings", "RecursivScan"));
    m_ui.showHiddenDirsCheckBox->setChecked(m_setupFile->getSectionBoolValue("Settings", "ShowHiddenDirs"));
    slot_hiddenFoldersCheckBoxClicked();
    m_ui.virusFoundComboBox->setCurrentIndex(m_setupFile->getSectionIntValue("Settings", "VirusFoundComboBox"));

    m_username = whoami();
    slot_updateDeviceList();
}


void scanTab::checkMonochromeSettings()
{
    if (m_monochrome == false)
    {
        m_devicelabel->setStyleSheet("background-color:#c0c0c0;color:black;padding:4px;border-radius:5px;");
        m_ui.pathLabel->setStyleSheet("background-color:#c0c0c0;color:black;padding:4px;border-radius:5px;");
    }
    else {
        m_devicelabel->setStyleSheet("background-color:#404040;color:white;padding:4px;border-radius:5px;");
        m_ui.pathLabel->setStyleSheet("background-color:#404040;color:white;padding:4px;border-radius:5px;");
    }
}

void scanTab::slot_scanButtonClicked()
{
    QList<QPersistentModelIndex> list = m_model->checkedIndexes().values();
    QStringList scanObjects;

    for (int i = 0; i < list.count(); i++)
    {
        if (list[i].data(QFileSystemModel::FilePathRole).toString() != "")
            scanObjects << list[i].data(QFileSystemModel::FilePathRole).toString();
    }

    if (scanObjects.count() > 0)
        emit triggerScanRequest(scanObjects);
}

void scanTab::slot_basePathButtonClicked()
{
    QModelIndex index = m_model->index("/");

    m_ui.treeView->scrollTo(index);
    m_ui.treeView->setCurrentIndex(index);
}

void scanTab::slot_homePathButtonClicked()
{
    QModelIndex index = m_model->index(QDir::homePath());

    m_ui.treeView->scrollTo(index);
    m_ui.treeView->setCurrentIndex(index);
}

void scanTab::slot_updateDeviceList()
{
    QDir dir;
    if (QFileInfo::exists("/run/media/" + m_username) == true)
        dir.setPath("/run/media/" + m_username);
    else
        if (QFileInfo::exists("/run/media") == true)
            dir.setPath("/run/media");
    if (QFileInfo::exists("/media/" + m_username) == true)
        dir.setPath("/media/" + m_username);
    else
        if (QFileInfo::exists("/media") == true)
            dir.setPath("/media/");

    if (m_fileSystemWatcher != nullptr) delete m_fileSystemWatcher;
    m_fileSystemWatcher = new QFileSystemWatcher(this);

    QStringList filters;
    filters << "*";

    QStringList dirs = dir.entryList(filters, QDir::AllDirs | QDir::NoDotAndDotDot);

    m_devicelabel = new QLabel(tr("Devices"));
    QLayoutItem* item = NULL;

    while ((item = m_ui.devicesFrame->layout()->takeAt(0)) != 0)
    {
        delete item->widget();
    }

    if (dir.path() != "")
    {
        m_fileSystemWatcher->addPath(dir.path());
        connect(m_fileSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(slot_updateDeviceList()));
    }

    if (m_setupFile->getSectionBoolValue("Setup", "DisableLogHighlighter") == true)
    {
        m_devicelabel->setStyleSheet("background-color:#404040;color:white;padding:3px;");
    }
    else {
        m_devicelabel->setStyleSheet("background-color:#c0c0c0;color:black;padding:3px;");
    }
    m_ui.devicesFrame->layout()->addWidget(m_devicelabel);

    m_deviceGroup = new QButtonGroup(this);
    connect(m_deviceGroup, SIGNAL(idClicked(int)), this, SLOT(slot_deviceButtonClicked(int)));

    if (dirs.count() > 0)
    {
        m_devices.clear();
        m_buttonID = 0;
        foreach (QString entry, dirs)
        {
            m_devices << (QString)(dir.path() + "/" + entry);
            dragablePushButton* button = new dragablePushButton(QIcon(":/icons/icons/media.png"), entry.mid(entry.lastIndexOf("/") + 1), this,
                                                                (QString)(dir.path() + "/" + entry));
            connect(button, SIGNAL(dragStarted()), this, SLOT(slot_requestDropZoneVisible()));
            button->setIconSize(QSize(28, 28));
            button->setStyleSheet("text-align:left");
            button->setFlat(true);
            m_deviceGroup->addButton(button, m_buttonID);
            m_ui.devicesFrame->layout()->addWidget(button);
            m_buttonID++;
        }
    }
}

void scanTab::slot_deviceButtonClicked(int buttonIndex)
{
    QModelIndex index = m_model->index(m_devices.at(buttonIndex));

    m_ui.treeView->scrollTo(index);
    m_ui.treeView->setCurrentIndex(index);
}

void scanTab::slot_recursivScanCheckBoxClicked()
{
    m_setupFile->setSectionValue("Settings", "RecursivScan", m_ui.recursivCheckBox->isChecked());
}

void scanTab::slot_virusFoundComboBoxChanged()
{
    m_setupFile->setSectionValue("Settings", "VirusFoundComboBox", m_ui.virusFoundComboBox->currentIndex());
}

bool scanTab::recursivChecked()
{
    return m_ui.recursivCheckBox->isChecked();
}

void scanTab::setStatusMessage(QString message)
{
    QString currentFile;
    int start, end;

    while (message.indexOf("Scanning") != -1)
    {
        start = message.indexOf("Scanning");
        end = message.indexOf("\n", start);
        currentFile = message.mid(start, end - start + 1);
        message.replace(currentFile, "");
        currentFile = currentFile.mid(currentFile.lastIndexOf("/") + 1);
        currentFile.replace("\n", "");
        m_ui.currentFileLabel->setText("Scanning : " + currentFile);
    }
    m_ui.logPlainTextEdit->insertPlainText(message);
    m_ui.logPlainTextEdit->ensureCursorVisible();
}

void scanTab::clearLogMessage()
{
    m_ui.logPlainTextEdit->setPlainText("");
}

void scanTab::slot_abortScan()
{
    m_ui.currentFileLabel->setText(tr("Scanning aborted ......"));
    if (m_movie != nullptr)
        m_movie->stop();
    emit abortScan();
}

void scanTab::slot_enableForm(bool mode)
{

    if (mode == false)
    {
        m_busyLabel = new QLabel(this);
        m_busyLabel->setStyleSheet("background:transparent");
        m_busyLabel->setGeometry((this->width() - 80) / 2, (this->height() - 80) / 2, 80, 80);
        m_movie = new QMovie(":/icons/icons/gifs/spinning_segments_large.gif");
        m_busyLabel->setMovie(m_movie);
        m_busyLabel->show();
        m_movie->start();
    }
    else {
        if (m_movie != 0)
        {
            m_movie->stop();
            delete m_movie;
            delete m_busyLabel;
            m_movie = nullptr;
            m_busyLabel = nullptr;
        }
    }

    m_ui.deviceFrame->setEnabled(mode);
    m_ui.startScanButton->setVisible(mode);
    m_ui.stopScanButton->setVisible(!mode);
    m_ui.uncheckAllPushButton->setEnabled(mode);
    m_ui.recursivCheckBox->setEnabled(mode);
    m_ui.showHiddenDirsCheckBox->setEnabled(mode);
    m_ui.virusFoundComboBox->setEnabled(mode);
    m_ui.treeView->setEnabled(mode);
}

void scanTab::setStatusBarMessage(QString message, QString bgColor)
{
    m_ui.currentFileLabel->setText(message);
    m_ui.currentFileLabel->setStyleSheet("background-color:" + bgColor);
}

int scanTab::getVirusFoundComboBoxValue()
{
    return m_ui.virusFoundComboBox->currentIndex();
}

void scanTab::slot_requestDropZoneVisible()
{
    emit requestDropZoneVisible();
}

void scanTab::slot_dirtreeSelectionChanged()
{
    QList<QPersistentModelIndex> list = m_model->checkedIndexes().values();
    QString directories;

    for (int i = 0; i < list.count(); i++)
    {
        if (i < list.count() - 1)
        {
            directories = directories + list[i].data(QFileSystemModel::FilePathRole).toString() + "\n";
        }
        else {
            directories = directories + list[i].data(QFileSystemModel::FilePathRole).toString();
        }
    }
    m_ui.selectedDirectoriesLabel->setText(directories);

    m_setupFile->setSectionValue("Settings", "Directories", directories);
}

void scanTab::slot_updateDBPath(QString path)
{
    m_setupFile->setSectionValue("Directories", "LoadSupportedDBFiles", "checked|" + path);
}

void scanTab::slot_disableScanButton()
{
    m_ui.startScanButton->setEnabled(false);
}

void scanTab::slot_hiddenFoldersCheckBoxClicked()
{
    if (m_ui.showHiddenDirsCheckBox->isChecked() == true)
    {
        m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Hidden);
    }
    else {
        m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    }
    m_setupFile->setSectionValue("Settings", "ShowHiddenDirs", m_ui.showHiddenDirsCheckBox->isChecked());
}

void scanTab::slot_add_remove_highlighter(bool state)
{
    m_monochrome = state;
    m_setupFile->setSectionValue("Setup", "DisableLogHighlighter", state);

    if (state == true)
    {
        if (m_logHighLighter != NULL)
        {
            delete m_logHighLighter;
            m_logHighLighter = NULL;
        }
    }
    else {
        if (m_logHighLighter == NULL)
        {
            m_logHighLighter = new highlighter(m_ui.logPlainTextEdit->document());
        }
        else {
            delete m_logHighLighter;
            m_logHighLighter = new highlighter(m_ui.logPlainTextEdit->document());
        }
    }
    checkMonochromeSettings();
}

void scanTab::slot_uncheckAllButtonClicked()
{
    m_setupFile->setSectionValue("Settings", "Directories", "");
    m_model->unCheckAll();
    m_ui.treeView->collapseAll();
    m_ui.selectedDirectoriesLabel->clear();
}
