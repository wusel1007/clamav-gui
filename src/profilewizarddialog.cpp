#include "profilewizarddialog.h"
#include "ui_profilewizarddialog.h"
#define css "background-color:#404040;color:white"

ProfileWizardDialog::ProfileWizardDialog(QWidget* parent, QString name) : QDialog(parent), m_profileName(name), m_ui(new Ui::ProfileWizardDialog)
{
    m_ui->setupUi(this);
    m_setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini", this);
    if (m_setupFile->keywordExists("Setup", "DisableLogHighlighter") == true)
        m_monochrome = m_setupFile->getSectionBoolValue("Setup", "DisableLogHighlighter");

    delete m_setupFile;

    if (m_profileName == "") {
        m_setupFileFilename = QDir::homePath() + "/.clamav-gui/settings.ini";
        m_setupFile = new setupFileHandler(m_setupFileFilename, this);
        m_newProfile = true;
    }
    else {
        m_setupFileFilename = QDir::homePath() + "/.clamav-gui/profiles/" + m_profileName + ".ini";
        m_setupFile = new setupFileHandler(m_setupFileFilename, this);
        m_ui->profileNameLineEdit->setReadOnly(true);
        m_ui->createButton->setText(tr("Save"));
        m_newProfile = false;
    }

    m_model = new CFileSystemModel;
    m_setupFile->getSectionBoolValue("Settings", "ShowHiddenDirs") == true ? m_model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden)
                                                                           : m_model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    m_ui->showHiddenDirsCheckBox->setChecked(m_setupFile->getSectionBoolValue("Settings", "ShowHiddenDirs"));
    m_model->setRootPath("/");
    m_model->unCheckAll();

    m_ui->treeView->setModel(m_model);
    m_ui->treeView->hideColumn(1);
    m_ui->treeView->hideColumn(2);
    m_ui->treeView->hideColumn(3);
    m_ui->treeView->collapseAll();

    m_profileName == "" ? m_ui->stackedWidget->setCurrentIndex(0) : m_ui->stackedWidget->setCurrentIndex(3);
    if (m_profileName != "") {
        m_ui->profileNameLineEdit->setText(m_profileName);
        readSettings();
    }

    slot_previousButtonClicked();

    m_getClamscanParametersProcess = new QProcess(this);
    connect(m_getClamscanParametersProcess, SIGNAL(readyReadStandardError()), this, SLOT(slot_getClamscanProcessHasOutput()));
    connect(m_getClamscanParametersProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(slot_getClamscanProcessHasOutput()));
    connect(m_getClamscanParametersProcess, SIGNAL(finished(int)), this, SLOT(slot_getClamscanProcessFinished()));

    QStringList parameters;
    parameters << "--help";

    m_getClamscanParametersProcess->start("clamscan", parameters);
}

ProfileWizardDialog::~ProfileWizardDialog()
{
    delete m_ui;
}

void ProfileWizardDialog::closeEvent(QCloseEvent*)
{
    QString profileFile = QDir::homePath() + "/.clamav-gui/profiles/" + m_ui->profileNameLineEdit->text() + ".ini";
    if ((m_newProfile == true) && (profileFile != QDir::homePath() + "/.clamav-gui/settings.ini")) {
        QFile killfile(profileFile);
        if (killfile.exists() == true)
            killfile.remove();
    }
}

void ProfileWizardDialog::readSettings()
{
    QString section = "REGEXP_and_IncludeExclude";
    QString keyword;
    QString value;
    QString checked;
    QString optionText;
    QString tooltipText;
    QStringList directories = m_setupFile->getSectionValue(m_ui->profileNameLineEdit->text(), "Directories").split("\n");
    QStringList availableOptions = m_setupFile->getKeywords("AvailableOptions");

    // DIRTREE
    m_ui->treeView->collapseAll();
    if (m_newProfile == true) {
        m_ui->recursivCheckBox->setChecked(m_setupFile->getSectionBoolValue("Settings", "RecursivScan"));
    }
    else {
        m_ui->recursivCheckBox->setChecked(m_setupFile->getSectionBoolValue(m_profileName, "Recursion"));
    }
    m_model->unCheckAll();
    foreach (QString dir, directories) {
        if (dir != "") {
            m_model->setChecked(dir, true);
            QModelIndex index = m_model->index(dir);
            m_ui->treeView->scrollTo(index);
            m_ui->treeView->setCurrentIndex(index);
        }
    }

    // DIRECTORIES
    value = m_setupFile->getSectionValue("Directories", "LoadSupportedDBFiles");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->loadVirusDatabaseCheckBox->setChecked(true) : m_ui->loadVirusDatabaseCheckBox->setChecked(false);
    m_ui->loadVirusDatabaseLineEdit->setText(value);

    value = m_setupFile->getSectionValue("Directories", "ScanReportToFile");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->scanReportToFileCheckBox->setChecked(true) : m_ui->scanReportToFileCheckBox->setChecked(false);
    m_ui->scanReportToFileLineEdit->setText(value);
    // use profile name as part of the report file for new profiles
    if(m_newProfile){
        QStringList pathSlices = value.split("/");
        value = value.replace(pathSlices[pathSlices.length() - 1], m_profileName + "_scanreport.log");
        m_ui->scanReportToFileLineEdit->setText(value);
    }

    value = m_setupFile->getSectionValue("Directories", "ScanFilesFromFile");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->scanFilesFromFileCheckBox->setChecked(true) : m_ui->scanFilesFromFileCheckBox->setChecked(false);
    m_ui->scanFilesFromFileLineEdit->setText(value);

    value = m_setupFile->getSectionValue("Directories", "TmpFile");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->tempFileCheckBox->setChecked(true) : m_ui->tempFileCheckBox->setChecked(false);
    m_ui->tempFilesLineEdit->setText(value);

    value = m_setupFile->getSectionValue("Directories", "MoveInfectedFiles");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->moveDirectoryCheckBox->setChecked(true) : m_ui->moveDirectoryCheckBox->setChecked(false);
    m_ui->moveDirectoryLineEdit->setText(value);

    value = m_setupFile->getSectionValue("Directories", "CopyInfectedFiles");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->copyDirectoryCheckBox->setChecked(true) : m_ui->copyDirectoryCheckBox->setChecked(false);
    m_ui->copyDirectoryLineEdit->setText(value);

    value = m_setupFile->getSectionValue("Directories", "FollowDirectorySymLinks");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->followDirectorySymlinksCheckBox->setChecked(true) : m_ui->followDirectorySymlinksCheckBox->setChecked(false);
    m_ui->followDirectorySymlinksComboBox->setCurrentIndex(value.toInt());

    value = m_setupFile->getSectionValue("Directories", "FollowFileSymLinks");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->followFileSymlinksCheckBox->setChecked(true) : m_ui->followFileSymlinksCheckBox->setChecked(false);
    m_ui->followFileSymlinksComboBox->setCurrentIndex(value.toInt());

    slot_directoryCheckBoxesClicked();

    // INCLUDE / EXCLUDE
    keyword = "DontScanFileNamesMatchingRegExp";
    value = m_setupFile->getSectionValue(section, keyword);
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwdontScanFileNameCheckBox->setChecked(true) : m_ui->pwdontScanFileNameCheckBox->setChecked(false);
    m_ui->pwdontScanFileNameLineEdit->setText(value);

    keyword = "DontScanDiretoriesMatchingRegExp";
    value = m_setupFile->getSectionValue(section, keyword);
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwdontScanDirCheckBox->setChecked(true) : m_ui->pwdontScanDirCheckBox->setChecked(false);
    m_ui->pwdontScanDirLineEdit->setText(value);

    keyword = "OnlyScanFileNamesMatchingRegExp";
    value = m_setupFile->getSectionValue(section, keyword);
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwonlyScanFileNameCheckBox->setChecked(true) : m_ui->pwonlyScanFileNameCheckBox->setChecked(false);
    m_ui->pwonlyScanFileNameLineEdit->setText(value);

    keyword = "OnlyScanDiretoriesMatchingRegExp";
    value = m_setupFile->getSectionValue(section, keyword);
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwonlyScanDirCheckBox->setChecked(true) : m_ui->pwonlyScanDirCheckBox->setChecked(false);
    m_ui->pwonlyScanDirLineEdit->setText(value);

    // PUA
    keyword = "EnablePUAOptions";
    if (m_setupFile->getSectionBoolValue(section, keyword) == true) {
        m_ui->pwenablePUACheckBox->setChecked(true);
        m_ui->pwPUAFrame->setEnabled(true);
    }
    else {
        m_ui->pwenablePUACheckBox->setChecked(false);
        m_ui->pwPUAFrame->setEnabled(false);
    }

    keyword = "LoadPUAPacked";
    m_ui->pwloadPUAPackedRadioButon->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    keyword = "LoadPUAPWTool";
    m_ui->pwloadPUAPWToolRadioButton->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    keyword = "LoadPUANetTool";
    m_ui->pwloadPUANetToolRadioButton->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    keyword = "LoadPUAP2P";
    m_ui->pwloadPUAP2PRadioButton->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    keyword = "LoadPUAIRC";
    m_ui->pwloadPUAIRCRadioButton->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    keyword = "LoadPUARAT";
    m_ui->pwloadPUARATRadioButton->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    keyword = "LoadPUANetToolSpy";
    m_ui->pwloadPUANetToolSpyRadioButton->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    keyword = "LoadPUAServer";
    m_ui->pwloadPUAServerRadioButton->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    keyword = "LoadPUAScript";
    m_ui->pwloadPUAScriptRadioButton->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    keyword = "LoadPUAAndr";
    m_ui->pwloadPUAAndrRadioButton->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    keyword = "LoadPUAJava";
    m_ui->pwloadPUAJavaRadioButton->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    keyword = "LoadPUAOsx";
    m_ui->pwloadPUAOsxRadioButton->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    keyword = "LoadPUATool";
    m_ui->pwloadPUAToolRadioButton->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    keyword = "LoadPUAUnix";
    m_ui->pwloadPUAUnixRadioButton->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    keyword = "LoadPUAWin";
    m_ui->pwloadPUAWinRadioButton->setChecked(m_setupFile->getSectionBoolValue(section, keyword));

    // SCANLIMITS
    value = m_setupFile->getSectionValue("ScanLimitations", "Files larger than this will be skipped and assumed clean");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")) {
        m_ui->pwfilesLargerThanThisComboBox->setCurrentIndex(m_ui->pwfilesLargerThanThisComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    else {
        m_ui->pwfilesLargerThanThisComboBox->setCurrentIndex(0);
    }
    checked == "checked" ? m_ui->pwfilesLargerThanThisCheckBox->setChecked(true) : m_ui->pwfilesLargerThanThisCheckBox->setChecked(false);
    m_ui->pwfilesLargerThanThisSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "The maximum amount of data to scan for each container file");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")) {
        m_ui->pwmaxAmountForContainerComboBox->setCurrentIndex(m_ui->pwmaxAmountForContainerComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    else {
        m_ui->pwmaxAmountForContainerComboBox->setCurrentIndex(0);
    }
    checked == "checked" ? m_ui->pwmaxAmountForContainerCheckBox->setChecked(true) : m_ui->pwmaxAmountForContainerCheckBox->setChecked(false);
    m_ui->pwmaxAmountForContainerSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "The maximum number of files to scan for each container file");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")) {
        m_ui->pwmaxNumberForContainerComboBox->setCurrentIndex(m_ui->pwmaxNumberForContainerComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    else {
        m_ui->pwmaxNumberForContainerComboBox->setCurrentIndex(0);
    }
    checked == "checked" ? m_ui->pwmaxNumberForContainerCheckBox->setChecked(true) : m_ui->pwmaxNumberForContainerCheckBox->setChecked(false);
    m_ui->pwmaxNumberForContainerSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Maximum archive recursion level for container file");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")) {
        m_ui->pwmaxArchiveRecursionForContainerComboBox->setCurrentIndex(m_ui->pwmaxArchiveRecursionForContainerComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    else {
        m_ui->pwmaxArchiveRecursionForContainerComboBox->setCurrentIndex(0);
    }
    checked == "checked" ? m_ui->pwmaxArchiveRecursionForContainerCheckBox->setChecked(true)
                         : m_ui->pwmaxArchiveRecursionForContainerCheckBox->setChecked(false);
    m_ui->pwmaxArchiveRecursionForContainerSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Maximum directory recursion level");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")) {
        m_ui->pwmaxDirRecursionLevelComboBox->setCurrentIndex(m_ui->pwmaxDirRecursionLevelComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    else {
        m_ui->pwmaxDirRecursionLevelComboBox->setCurrentIndex(0);
    }
    checked == "checked" ? m_ui->pwmaxDirRecursionLevelCheckBox->setChecked(true) : m_ui->pwmaxDirRecursionLevelCheckBox->setChecked(false);
    m_ui->pwmaxDirRecursionLevelSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Maximum size file to check for embedded PE");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")) {
        m_ui->pwmaxSizeFileForPEComboBox->setCurrentIndex(m_ui->pwmaxSizeFileForPEComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    else {
        m_ui->pwmaxSizeFileForPEComboBox->setCurrentIndex(0);
    }
    checked == "checked" ? m_ui->pwmaxSizeFileForPECheckBox->setChecked(true) : m_ui->pwmaxSizeFileForPECheckBox->setChecked(false);
    m_ui->pwmaxSizeFileForPESpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Maximum size of HTML file to normalize");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")) {
        m_ui->pwmaxSizeHTMLFileToNormalizeComboBox->setCurrentIndex(m_ui->pwmaxSizeHTMLFileToNormalizeComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    else {
        m_ui->pwmaxSizeHTMLFileToNormalizeComboBox->setCurrentIndex(0);
    }
    checked == "checked" ? m_ui->pwmaxSizeHTMLFileToNormalizeCheckBox->setChecked(true)
                         : m_ui->pwmaxSizeHTMLFileToNormalizeCheckBox->setChecked(false);
    m_ui->pwmaxSizeHTMLFileToNormalizeSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Maximum size of normalized HTML file to scan");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")) {
        m_ui->pwmaxSizeOfNormalizedHTMLFileComboBox->setCurrentIndex(m_ui->pwmaxSizeOfNormalizedHTMLFileComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    else {
        m_ui->pwmaxSizeOfNormalizedHTMLFileComboBox->setCurrentIndex(0);
    }
    checked == "checked" ? m_ui->pwmaxSizeOfNormalizedHTMLFileCheckBox->setChecked(true)
                         : m_ui->pwmaxSizeOfNormalizedHTMLFileCheckBox->setChecked(false);
    m_ui->pwmaxSizeOfNormalizedHTMLFileSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Maximum size of script file to normalize");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")) {
        m_ui->pwmaxSizeOfScriptFileToNormalizeComboBox->setCurrentIndex(m_ui->pwmaxSizeOfScriptFileToNormalizeComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    else {
        m_ui->pwmaxSizeOfScriptFileToNormalizeComboBox->setCurrentIndex(0);
    }
    checked == "checked" ? m_ui->pwmaxSizeOfScriptFileToNormalizeCheckBox->setChecked(true)
                         : m_ui->pwmaxSizeOfScriptFileToNormalizeCheckBox->setChecked(false);
    m_ui->pwmaxSizeOfScriptFileToNormalizeSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Maximum size zip to type reanalyze");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")) {
        m_ui->pwmaxSizeZipToTypeReanalzeComboBox->setCurrentIndex(m_ui->pwmaxSizeZipToTypeReanalzeComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    else {
        m_ui->pwmaxSizeZipToTypeReanalzeComboBox->setCurrentIndex(0);
    }
    checked == "checked" ? m_ui->pwmaxSizeZipToTypeReanalzeCheckBox->setChecked(true) : m_ui->pwmaxSizeZipToTypeReanalzeCheckBox->setChecked(false);
    m_ui->pwmaxSizeZipToTypeReanalzeSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Maximum number of partitions in disk image to be scanned");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")) {
        m_ui->pwmaxNumberOfPartitionsInDiskImageComboBox->setCurrentIndex(m_ui->pwmaxNumberOfPartitionsInDiskImageComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    else {
        m_ui->pwmaxNumberOfPartitionsInDiskImageComboBox->setCurrentIndex(0);
    }
    checked == "checked" ? m_ui->pwmaxNumberOfPartitionsInDiskImageCheckBox->setChecked(true)
                         : m_ui->pwmaxNumberOfPartitionsInDiskImageCheckBox->setChecked(false);
    m_ui->pwmaxNumberOfPartitionsInDiskImageSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Maximum number of icons in PE file to be scanned");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")) {
        m_ui->pwmaxNumberOfIconsInPEFileComboBox->setCurrentIndex(m_ui->pwmaxNumberOfIconsInPEFileComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    else {
        m_ui->pwmaxNumberOfIconsInPEFileComboBox->setCurrentIndex(0);
    }
    checked == "checked" ? m_ui->pwmaxNumberOfIconsInPEFileCheckBox->setChecked(true) : m_ui->pwmaxNumberOfIconsInPEFileCheckBox->setChecked(false);
    m_ui->pwmaxNumberOfIconsInPEFileSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Number of seconds to wait for waiting a response back from the stats server");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwnumberOfSecondsForResponseCheckBox->setChecked(true)
                         : m_ui->pwnumberOfSecondsForResponseCheckBox->setChecked(false);
    m_ui->pwnumberOfSecondsForResponseSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Bytecode timeout in milliseconds");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwBytecodeTimeoutCheckBox->setChecked(true) : m_ui->pwBytecodeTimeoutCheckBox->setChecked(false);
    m_ui->pwBytecodeTimeoutSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Collect and print execution statistics");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwExecutionStatisticsCheckBox->setChecked(true) : m_ui->pwExecutionStatisticsCheckBox->setChecked(false);
    m_ui->pwExecutionStatisticsComboBox->setCurrentIndex(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Structured SSN Format");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwstructuredSSNFormatCheckBox->setChecked(true) : m_ui->pwstructuredSSNFormatCheckBox->setChecked(false);
    m_ui->pwStructuredSSNFormatComboBox->setCurrentIndex(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Structured SSN Count");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwStructuredSSNCountCheckBox->setChecked(true) : m_ui->pwStructuredSSNCountCheckBox->setChecked(false);
    m_ui->pwStructuredSSNCountSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Structured CC Count");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwStructuredCCCountCheckBox->setChecked(true) : m_ui->pwStructuredCCCountCheckBox->setChecked(false);
    m_ui->pwStructuredCCCountSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Structured CC Mode");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwStructuredCCModeCheckBox->setChecked(true) : m_ui->pwStructuredCCModeCheckBox->setChecked(false);
    m_ui->pwStructuredCCModeComboBox->setCurrentIndex(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Max Scan-Time");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwMaxScanTimeCheckBox->setChecked(true) : m_ui->pwMaxScanTimeCheckBox->setChecked(false);
    m_ui->pwMaxScanTimeSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Max recursion to HWP3 parsing function");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwMaxRecursionHWP3CheckBox->setChecked(true) : m_ui->pwMaxRecursionHWP3CheckBox->setChecked(false);
    m_ui->pwMaxRecursionHWP3SpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Max calls to PCRE match function");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwMaxCallsPCREMatchFunctionCheckBox->setChecked(true) : m_ui->pwMaxCallsPCREMatchFunctionCheckBox->setChecked(false);
    m_ui->pwMaxCallsPCREMatchFunctionSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Max recursion calls to the PCRE match function");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwMaxRecursionCallsPCREMatchFunctionCheckBox->setChecked(true)
                         : m_ui->pwMaxRecursionCallsPCREMatchFunctionCheckBox->setChecked(false);
    m_ui->pwMaxRecursionCallsPCREMatchFunctionCpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Max PCRE file size");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")) {
        m_ui->pwMaxPCREFileSizeComboBox->setCurrentIndex(m_ui->pwMaxPCREFileSizeComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked == "checked" ? m_ui->pwMaxPCREFileSizeCheckBox->setChecked(true) : m_ui->pwMaxPCREFileSizeCheckBox->setChecked(false);
    m_ui->pwMaxPCREFileSizeSpinBox->setValue(value.toInt());

    value = m_setupFile->getSectionValue("ScanLimitations", "Database outdated if older than x days");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked == "checked" ? m_ui->pwdatabaseOutdatedCheckbox->setChecked(true) : m_ui->pwdatabaseOutdatedCheckbox->setChecked(false);
    m_ui->pwdatabaseOutdatedSpinBox->setValue(value.toInt());

    // OPTIONS

    QString lastOption = "xyz";
    QString language = setupFileHandler::getSectionValue(QDir::homePath() + "/.clamav-gui/settings.ini","Setup","language");
    scanoption* option;
    scanoptionyn* optionyn;
    QString newProfilename = QDir::homePath() + "/.clamav-gui/profiles/" + m_profileName + ".ini";
    setupFileHandler* tempconfig = new setupFileHandler(newProfilename, this);
    if (language == "") language = "[en_GB]";

    if (m_ui->optionLayout->count() == 0) {
        for (int i = 0; i < availableOptions.count(); i++) {
            optionText = availableOptions.at(i);
            tooltipText = m_setupFile->getSectionValue("AvailableOptions", optionText);
            tempconfig->setSectionValue("AvailableOptions", optionText, tooltipText);
            if (optionText.indexOf(lastOption) == -1) {
                if (optionText.indexOf("<equal>") != -1)
                    lastOption = optionText.left(optionText.indexOf("<equal>"));
                else
                    lastOption = optionText;
                if (optionText.indexOf("<equal>") != -1) {
                    // --Switches with yes/no option
                    QString keyword = optionText.left(optionText.indexOf("<equal>"));
                    QString yes_no = optionText.mid(optionText.indexOf("<equal>") + 7);
                    bool optionfound = false;
                    if (m_setupFile->keywordExists("SelectedOptions", keyword + "<equal>yes") == true) {
                        optionyn = new scanoptionyn(this, newProfilename, "SelectedOptions", true, keyword + "<equal>yes",
                                                    m_setupFile->getSectionValue("AvailableOptions", optionText),language);
                        optionfound = true;
                    }
                    if ((m_setupFile->keywordExists("SelectedOptions", keyword + "<equal>no") == true) && (optionfound == false)) {
                        optionyn = new scanoptionyn(this, newProfilename, "SelectedOptions", true, keyword + "<equal>no",
                                                    m_setupFile->getSectionValue("AvailableOptions", optionText),language);
                        optionfound = true;
                    }
                    if (optionfound == false) {
                        optionyn = new scanoptionyn(this, newProfilename, "SelectedOptions", false, keyword + "<equal>" + yes_no,
                                                    m_setupFile->getSectionValue("AvailableOptions", optionText),language);
                    }
                    m_ui->optionLayout->addWidget(optionyn);
                }
                else {
                    // --Switches without yes/no
                    if (m_setupFile->keywordExists("SelectedOptions", optionText) == true) {
                        option = new scanoption(this, newProfilename, "SelectedOptions", true, optionText,
                                                m_setupFile->getSectionValue("AvailableOptions", optionText),language);
                    }
                    else {
                        option = new scanoption(this, newProfilename, "SelectedOptions", false, optionText,
                                                m_setupFile->getSectionValue("AvailableOptions", optionText),language);
                    }
                    m_ui->optionLayout->addWidget(option);
                }
            }
        }
    }
    slot_scanLimitsCheckBoxClicked();
    slot_highlightSettings();
}

void ProfileWizardDialog::slot_nextButtonClicked()
{
    int index = m_ui->stackedWidget->currentIndex();
    int count = m_ui->stackedWidget->count() - 1;

    if (index < count) {
        index++;
        m_ui->stackedWidget->setCurrentIndex(index);
    }

    if ((m_ui->profileNameLineEdit->text() != "") && (m_ui->optionLayout->count() == 0)) {
        m_profileName = m_ui->profileNameLineEdit->text();
        readSettings();
    }

    slot_checkNavigationButtons();
}

void ProfileWizardDialog::slot_previousButtonClicked()
{
    int index = m_ui->stackedWidget->currentIndex();

    if (index > 0) {
        index--;
        m_ui->stackedWidget->setCurrentIndex(index);
    }
    slot_checkNavigationButtons();
}

void ProfileWizardDialog::slot_checkNavigationButtons()
{
    int index = m_ui->stackedWidget->currentIndex();

    QFile checkFile(QDir::homePath() + "/.clamav-gui/profiles/" + m_ui->profileNameLineEdit->text() + ".ini");

    switch (index) {
        case 0:
            m_ui->previousButton->setEnabled(false);
            break;
        case 1:
            m_ui->previousButton->setEnabled(true);
            if ((m_ui->profileNameLineEdit->text() == "") | (checkFile.exists()))
                m_ui->nextButton->setEnabled(false);
            else
                m_ui->nextButton->setEnabled(true);
            if (m_profileName != "")
                m_ui->nextButton->setEnabled(true);
            break;
        case 2:
            slot_listChanged();
            if (m_profileName != "")
                m_ui->previousButton->setEnabled(false);
            break;
        default:
            m_ui->previousButton->setEnabled(true);
            if (index == m_ui->stackedWidget->count() - 1)
                m_ui->nextButton->setEnabled(false);
            else
                m_ui->nextButton->setEnabled(true);
            if (index == m_ui->stackedWidget->count() - 1)
                m_ui->createButton->setEnabled(true);
            else
                m_ui->createButton->setEnabled(false);
            break;
    }
}

void ProfileWizardDialog::slot_enablePUACheckBoxActivated()
{
    m_ui->pwenablePUACheckBox->isChecked() ? m_ui->pwPUAFrame->setEnabled(true) : m_ui->pwPUAFrame->setEnabled(false);
}

void ProfileWizardDialog::slot_homeButtonClicked()
{
    QModelIndex index = m_model->index(QDir::homePath());

    m_ui->treeView->scrollTo(index);
    m_ui->treeView->setCurrentIndex(index);
}

void ProfileWizardDialog::slot_rootButtonClicked()
{
    QModelIndex index = m_model->index("/");

    m_ui->treeView->scrollTo(index);
    m_ui->treeView->setCurrentIndex(index);
}

void ProfileWizardDialog::slot_listChanged()
{
    QList<QPersistentModelIndex> list = m_model->checkedIndexes().values();

    list.count() == 0 ? m_ui->nextButton->setEnabled(false) : m_ui->nextButton->setEnabled(true);
}

void ProfileWizardDialog::slot_loadAllSupportedDBButtonClicked()
{
    QString folder = QFileDialog::getExistingDirectory(this, tr("Signature Folder"), m_ui->loadVirusDatabaseLineEdit->text());
    if (folder != "")
        m_ui->loadVirusDatabaseLineEdit->setText(folder);
}

void ProfileWizardDialog::slot_saveScanReportToFileButtonClicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Log-File"), m_ui->scanReportToFileLineEdit->text());
    if (filename != "")
        m_ui->scanReportToFileLineEdit->setText(filename);
}

void ProfileWizardDialog::slot_scanFromFileButtonClicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Scan from File"), m_ui->scanFilesFromFileLineEdit->text());
    if (filename != "")
        m_ui->scanFilesFromFileLineEdit->setText(filename);
}

void ProfileWizardDialog::slot_createTempFilesButtonClicked()
{
    QString folder = QFileDialog::getExistingDirectory(this, tr("Folder for temporary files"), m_ui->tempFilesLineEdit->text());
    if (folder != "")
        m_ui->tempFilesLineEdit->setText(folder);
}

void ProfileWizardDialog::slot_moveInfectedFilesButtonClicked()
{
    QString folder = QFileDialog::getExistingDirectory(this, tr("Move Folder for infected Files"), m_ui->moveDirectoryLineEdit->text());
    if (folder != "")
        m_ui->moveDirectoryLineEdit->setText(folder);
}

void ProfileWizardDialog::slot_copyInfectedFilesButtonClicked()
{
    QString folder = QFileDialog::getExistingDirectory(this, tr("Copy Folder for infected Files"), m_ui->copyDirectoryLineEdit->text());
    if (folder != "")
        m_ui->copyDirectoryLineEdit->setText(folder);
}

void ProfileWizardDialog::slot_createButtonClicked()
{
    QList<QPersistentModelIndex> list = m_model->checkedIndexes().values();
    QString directories;
    QString profileName = m_ui->profileNameLineEdit->text();
    setupFileHandler* profiles = new setupFileHandler(QDir::homePath() + "/.clamav-gui/profiles/" + profileName + ".ini", this);
    QString section = "REGEXP_and_IncludeExclude";
    QString checked;
    QString keyword;
    QString value;
    QString optionString;
    QString tooltipString;

    m_newProfile = false;
    for (int i = 0; i < list.count(); i++) {
        if (i < list.count() - 1) {
            directories = directories + list[i].data(QFileSystemModel::FilePathRole).toString() + "\n";
        }
        else {
            directories = directories + list[i].data(QFileSystemModel::FilePathRole).toString();
        }
    }

    profiles->setSectionValue(profileName, "Directories", directories);
    profiles->setSectionValue(profileName, "Recursion", m_ui->recursivCheckBox->isChecked());
    m_setupFile->setSectionValue("Profiles", profileName, profileName);

    keyword = "LoadSupportedDBFiles";
    value = m_ui->loadVirusDatabaseLineEdit->text();
    m_ui->loadVirusDatabaseCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("Directories", keyword, checked + "|" + value);

    keyword = "ScanReportToFile";
    value = m_ui->scanReportToFileLineEdit->text();
    m_ui->scanReportToFileCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("Directories", keyword, checked + "|" + value);

    keyword = "ScanFilesFromFile";
    value = m_ui->scanFilesFromFileLineEdit->text();
    m_ui->scanFilesFromFileCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("Directories", keyword, checked + "|" + value);

    keyword = "TmpFile";
    value = m_ui->tempFilesLineEdit->text();
    m_ui->tempFileCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("Directories", keyword, checked + "|" + value);

    keyword = "MoveInfectedFiles";
    value = m_ui->moveDirectoryLineEdit->text();
    m_ui->moveDirectoryCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("Directories", keyword, checked + "|" + value);

    keyword = "CopyInfectedFiles";
    value = m_ui->copyDirectoryLineEdit->text();
    m_ui->copyDirectoryCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("Directories", keyword, checked + "|" + value);

    keyword = "FollowDirectorySymLinks";
    value = QString::number(m_ui->followDirectorySymlinksComboBox->currentIndex());
    m_ui->followDirectorySymlinksCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("Directories", keyword, checked + "|" + value);

    keyword = "FollowFileSymLinks";
    value = QString::number(m_ui->followFileSymlinksComboBox->currentIndex());
    m_ui->followFileSymlinksCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("Directories", keyword, checked + "|" + value);

    //    profiles->removeSection("AvailableOptions");
    //    profiles->removeSection("SelectedOptions");

    keyword = "DontScanFileNamesMatchingRegExp";
    value = m_ui->pwdontScanFileNameLineEdit->text();
    m_ui->pwdontScanFileNameCheckBox->isChecked() ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue(section, keyword, checked + "|" + value);

    keyword = "DontScanDiretoriesMatchingRegExp";
    value = m_ui->pwdontScanDirLineEdit->text();
    m_ui->pwdontScanDirCheckBox->isChecked() ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue(section, keyword, checked + "|" + value);

    keyword = "OnlyScanFileNamesMatchingRegExp";
    value = m_ui->pwonlyScanFileNameLineEdit->text();
    m_ui->pwonlyScanFileNameCheckBox->isChecked() ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue(section, keyword, checked + "|" + value);

    keyword = "OnlyScanDiretoriesMatchingRegExp";
    value = m_ui->pwonlyScanDirLineEdit->text();
    m_ui->pwonlyScanDirCheckBox->isChecked() ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue(section, keyword, checked + "|" + value);

    keyword = "EnablePUAOptions";
    profiles->setSectionValue(section, keyword, m_ui->pwenablePUACheckBox->isChecked());

    keyword = "LoadPUAPacked";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUAPackedRadioButon->isChecked());

    keyword = "LoadPUAPWTool";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUAPWToolRadioButton->isChecked());

    keyword = "LoadPUANetTool";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUANetToolRadioButton->isChecked());

    keyword = "LoadPUAP2P";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUAP2PRadioButton->isChecked());

    keyword = "LoadPUAIRC";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUAIRCRadioButton->isChecked());

    keyword = "LoadPUARAT";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUARATRadioButton->isChecked());

    keyword = "LoadPUANetToolSpy";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUANetToolSpyRadioButton->isChecked());

    keyword = "LoadPUAServer";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUAServerRadioButton->isChecked());

    keyword = "LoadPUAScript";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUAScriptRadioButton->isChecked());

    keyword = "LoadPUAAndr";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUAAndrRadioButton->isChecked());

    keyword = "LoadPUAJava";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUAJavaRadioButton->isChecked());

    keyword = "LoadPUAOsx";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUAOsxRadioButton->isChecked());

    keyword = "LoadPUATool";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUAToolRadioButton->isChecked());

    keyword = "LoadPUAUnix";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUAUnixRadioButton->isChecked());

    keyword = "LoadPUAWin";
    profiles->setSectionValue(section, keyword, m_ui->pwloadPUAWinRadioButton->isChecked());

    keyword = "Files larger than this will be skipped and assumed clean";
    value = QString::number(m_ui->pwfilesLargerThanThisSpinBox->value()) + m_ui->pwfilesLargerThanThisComboBox->currentText();
    m_ui->pwfilesLargerThanThisCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "The maximum amount of data to scan for each container file";
    value = QString::number(m_ui->pwmaxAmountForContainerSpinBox->value()) + m_ui->pwmaxAmountForContainerComboBox->currentText();
    m_ui->pwmaxAmountForContainerCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "The maximum number of files to scan for each container file";
    value = QString::number(m_ui->pwmaxNumberForContainerSpinBox->value()) + m_ui->pwmaxNumberForContainerComboBox->currentText();
    m_ui->pwmaxNumberForContainerCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Maximum archive recursion level for container file";
    value = QString::number(m_ui->pwmaxArchiveRecursionForContainerSpinBox->value()) + m_ui->pwmaxArchiveRecursionForContainerComboBox->currentText();
    m_ui->pwmaxArchiveRecursionForContainerCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Maximum directory recursion level";
    value = QString::number(m_ui->pwmaxDirRecursionLevelSpinBox->value()) + m_ui->pwmaxDirRecursionLevelComboBox->currentText();
    m_ui->pwmaxDirRecursionLevelCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Maximum size file to check for embedded PE";
    value = QString::number(m_ui->pwmaxSizeFileForPESpinBox->value()) + m_ui->pwmaxSizeFileForPEComboBox->currentText();
    m_ui->pwmaxSizeFileForPECheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Maximum size of HTML file to normalize";
    value = QString::number(m_ui->pwmaxSizeHTMLFileToNormalizeSpinBox->value()) + m_ui->pwmaxSizeHTMLFileToNormalizeComboBox->currentText();
    m_ui->pwmaxSizeHTMLFileToNormalizeCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Maximum size of normalized HTML file to scan";
    value = QString::number(m_ui->pwmaxSizeOfNormalizedHTMLFileSpinBox->value()) + m_ui->pwmaxSizeOfNormalizedHTMLFileComboBox->currentText();
    m_ui->pwmaxSizeOfNormalizedHTMLFileCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Maximum size of script file to normalize";
    value = QString::number(m_ui->pwmaxSizeOfScriptFileToNormalizeSpinBox->value()) + m_ui->pwmaxSizeOfScriptFileToNormalizeComboBox->currentText();
    m_ui->pwmaxSizeOfScriptFileToNormalizeCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Maximum size zip to type reanalyze";
    value = QString::number(m_ui->pwmaxSizeZipToTypeReanalzeSpinBox->value()) + m_ui->pwmaxSizeZipToTypeReanalzeComboBox->currentText();
    m_ui->pwmaxSizeZipToTypeReanalzeCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Maximum number of partitions in disk image to be scanned";
    value =
        QString::number(m_ui->pwmaxNumberOfPartitionsInDiskImageSpinBox->value()) + m_ui->pwmaxNumberOfPartitionsInDiskImageComboBox->currentText();
    m_ui->pwmaxNumberOfPartitionsInDiskImageCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Maximum number of icons in PE file to be scanned";
    value = QString::number(m_ui->pwmaxNumberOfIconsInPEFileSpinBox->value()) + m_ui->pwmaxNumberOfIconsInPEFileComboBox->currentText();
    m_ui->pwmaxNumberOfIconsInPEFileCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Number of seconds to wait for waiting a response back from the stats server";
    value = QString::number(m_ui->pwnumberOfSecondsForResponseSpinBox->value());
    m_ui->pwnumberOfSecondsForResponseCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Bytecode timeout in milliseconds";
    value = QString::number(m_ui->pwBytecodeTimeoutSpinBox->value());
    m_ui->pwBytecodeTimeoutCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Collect and print execution statistics";
    value = QString::number(m_ui->pwExecutionStatisticsComboBox->currentIndex());
    m_ui->pwExecutionStatisticsCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Structured SSN Format";
    value = QString::number(m_ui->pwStructuredSSNFormatComboBox->currentIndex());
    m_ui->pwstructuredSSNFormatCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Structured SSN Count";
    value = QString::number(m_ui->pwStructuredSSNCountSpinBox->value());
    m_ui->pwStructuredSSNCountCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Structured CC Count";
    value = QString::number(m_ui->pwStructuredCCCountSpinBox->value());
    m_ui->pwStructuredCCCountCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Structured CC Mode";
    value = QString::number(m_ui->pwStructuredCCModeComboBox->currentIndex());
    m_ui->pwStructuredCCModeCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Max Scan-Time";
    value = QString::number(m_ui->pwMaxScanTimeSpinBox->value());
    m_ui->pwMaxScanTimeCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Max recursion to HWP3 parsing function";
    value = QString::number(m_ui->pwMaxRecursionHWP3SpinBox->value());
    m_ui->pwMaxRecursionHWP3CheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Max calls to PCRE match function";
    value = QString::number(m_ui->pwMaxCallsPCREMatchFunctionSpinBox->value());
    m_ui->pwMaxCallsPCREMatchFunctionCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Max recursion calls to the PCRE match function";
    value = QString::number(m_ui->pwMaxRecursionCallsPCREMatchFunctionCpinBox->value());
    m_ui->pwMaxRecursionCallsPCREMatchFunctionCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Max PCRE file size";
    value = QString::number(m_ui->pwMaxPCREFileSizeSpinBox->value()) + m_ui->pwMaxPCREFileSizeComboBox->currentText();
    m_ui->pwMaxPCREFileSizeCheckBox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    keyword = "Database outdated if older than x days";
    value = QString::number(m_ui->pwdatabaseOutdatedSpinBox->value());
    m_ui->pwdatabaseOutdatedCheckbox->isChecked() == true ? checked = "checked" : checked = "not checked";
    profiles->setSectionValue("ScanLimitations", keyword, checked + "|" + value);

    emit signal_profileSaved();
    this->accept();
}

void ProfileWizardDialog::slot_getClamscanProcessHasOutput()
{
    m_getClamscanProcessOutput = m_getClamscanProcessOutput + m_getClamscanParametersProcess->readAll();
}

void ProfileWizardDialog::slot_getClamscanProcessFinished()
{
    m_getClamscanProcessOutput = m_getClamscanProcessOutput + m_getClamscanParametersProcess->readAll();

    m_ui->pwBytecodeTimeoutCheckBox->setVisible(false);
    m_ui->pwBytecodeTimeoutSpinBox->setVisible(false);

    m_ui->pwExecutionStatisticsCheckBox->setVisible(false);
    m_ui->pwExecutionStatisticsComboBox->setVisible(false);

    m_ui->pwMaxCallsPCREMatchFunctionCheckBox->setVisible(false);
    m_ui->pwMaxCallsPCREMatchFunctionSpinBox->setVisible(false);

    m_ui->pwMaxPCREFileSizeCheckBox->setVisible(false);
    m_ui->pwMaxPCREFileSizeComboBox->setVisible(false);
    m_ui->pwMaxPCREFileSizeSpinBox->setVisible(false);

    m_ui->pwMaxRecursionCallsPCREMatchFunctionCheckBox->setVisible(false);
    m_ui->pwMaxRecursionCallsPCREMatchFunctionCpinBox->setVisible(false);

    m_ui->pwMaxRecursionHWP3CheckBox->setVisible(false);
    m_ui->pwMaxRecursionHWP3SpinBox->setVisible(false);

    m_ui->pwMaxScanTimeCheckBox->setVisible(false);
    m_ui->pwMaxScanTimeSpinBox->setVisible(false);

    m_ui->pwStructuredCCCountCheckBox->setVisible(false);
    m_ui->pwStructuredCCCountSpinBox->setVisible(false);

    m_ui->pwStructuredCCModeCheckBox->setVisible(false);
    m_ui->pwStructuredCCModeComboBox->setVisible(false);

    m_ui->pwStructuredSSNCountCheckBox->setVisible(false);
    m_ui->pwStructuredSSNCountSpinBox->setVisible(false);

    m_ui->pwstructuredSSNFormatCheckBox->setVisible(false);
    m_ui->pwStructuredSSNFormatComboBox->setVisible(false);

    m_ui->pwdatabaseOutdatedCheckbox->setVisible(false);
    m_ui->pwdatabaseOutdatedSpinBox->setVisible(false);

    m_ui->pwfilesLargerThanThisCheckBox->setVisible(false);
    m_ui->pwfilesLargerThanThisComboBox->setVisible(false);
    m_ui->pwfilesLargerThanThisSpinBox->setVisible(false);

    m_ui->pwmaxAmountForContainerCheckBox->setVisible(false);
    m_ui->pwmaxAmountForContainerComboBox->setVisible(false);
    m_ui->pwmaxAmountForContainerSpinBox->setVisible(false);

    m_ui->pwmaxArchiveRecursionForContainerCheckBox->setVisible(false);
    m_ui->pwmaxArchiveRecursionForContainerComboBox->setVisible(false);
    m_ui->pwmaxArchiveRecursionForContainerSpinBox->setVisible(false);

    m_ui->pwmaxDirRecursionLevelCheckBox->setVisible(false);
    m_ui->pwmaxDirRecursionLevelComboBox->setVisible(false);
    m_ui->pwmaxDirRecursionLevelSpinBox->setVisible(false);

    m_ui->pwmaxNumberForContainerCheckBox->setVisible(false);
    m_ui->pwmaxNumberForContainerComboBox->setVisible(false);
    m_ui->pwmaxNumberForContainerSpinBox->setVisible(false);

    m_ui->pwmaxNumberOfIconsInPEFileCheckBox->setVisible(false);
    m_ui->pwmaxNumberOfIconsInPEFileComboBox->setVisible(false);
    m_ui->pwmaxNumberOfIconsInPEFileSpinBox->setVisible(false);

    m_ui->pwmaxNumberOfPartitionsInDiskImageCheckBox->setVisible(false);
    m_ui->pwmaxNumberOfPartitionsInDiskImageComboBox->setVisible(false);
    m_ui->pwmaxNumberOfPartitionsInDiskImageSpinBox->setVisible(false);

    m_ui->pwmaxSizeFileForPECheckBox->setVisible(false);
    m_ui->pwmaxSizeFileForPEComboBox->setVisible(false);
    m_ui->pwmaxSizeFileForPESpinBox->setVisible(false);

    m_ui->pwmaxSizeHTMLFileToNormalizeCheckBox->setVisible(false);
    m_ui->pwmaxSizeHTMLFileToNormalizeComboBox->setVisible(false);
    m_ui->pwmaxSizeHTMLFileToNormalizeSpinBox->setVisible(false);

    m_ui->pwmaxSizeOfNormalizedHTMLFileCheckBox->setVisible(false);
    m_ui->pwmaxSizeOfNormalizedHTMLFileComboBox->setVisible(false);
    m_ui->pwmaxSizeOfNormalizedHTMLFileSpinBox->setVisible(false);

    m_ui->pwmaxSizeOfScriptFileToNormalizeCheckBox->setVisible(false);
    m_ui->pwmaxSizeOfScriptFileToNormalizeComboBox->setVisible(false);
    m_ui->pwmaxSizeOfScriptFileToNormalizeSpinBox->setVisible(false);

    m_ui->pwmaxSizeZipToTypeReanalzeCheckBox->setVisible(false);
    m_ui->pwmaxSizeZipToTypeReanalzeComboBox->setVisible(false);
    m_ui->pwmaxSizeZipToTypeReanalzeSpinBox->setVisible(false);

    m_ui->pwnumberOfSecondsForResponseCheckBox->setVisible(false);
    m_ui->pwnumberOfSecondsForResponseSpinBox->setVisible(false);
    m_ui->limitFrame_13->setVisible(false);

    QStringList lines = m_getClamscanProcessOutput.split("\n");
    QString line;

    for (int x = 0; x < lines.size(); x++) {
        line = lines[x];
        if (line.indexOf("--bytecode-timeout") != -1) {
            m_ui->pwBytecodeTimeoutCheckBox->setVisible(true);
            m_ui->pwBytecodeTimeoutSpinBox->setVisible(true);
        }

        if (line.indexOf("--statistics") != -1) {
            m_ui->pwExecutionStatisticsCheckBox->setVisible(true);
            m_ui->pwExecutionStatisticsComboBox->setVisible(true);
        }

        if (line.indexOf("--pcre-match-limit") != -1) {
            m_ui->pwMaxCallsPCREMatchFunctionCheckBox->setVisible(true);
            m_ui->pwMaxCallsPCREMatchFunctionSpinBox->setVisible(true);
        }

        if (line.indexOf("--pcre-max-filesize") != -1) {
            m_ui->pwMaxPCREFileSizeCheckBox->setVisible(true);
            m_ui->pwMaxPCREFileSizeComboBox->setVisible(true);
            m_ui->pwMaxPCREFileSizeSpinBox->setVisible(true);
        }

        if (line.indexOf("--pcre-recmatch-limit") != -1) {
            m_ui->pwMaxRecursionCallsPCREMatchFunctionCheckBox->setVisible(true);
            m_ui->pwMaxRecursionCallsPCREMatchFunctionCpinBox->setVisible(true);
        }

        if (line.indexOf("--max-rechwp3") != -1) {
            m_ui->pwMaxRecursionHWP3CheckBox->setVisible(true);
            m_ui->pwMaxRecursionHWP3SpinBox->setVisible(true);
        }

        if (line.indexOf("--max-scantime") != -1) {
            m_ui->pwMaxScanTimeCheckBox->setVisible(true);
            m_ui->pwMaxScanTimeSpinBox->setVisible(true);
        }

        if (line.indexOf("--structured-cc-count") != -1) {
            m_ui->pwStructuredCCCountCheckBox->setVisible(true);
            m_ui->pwStructuredCCCountSpinBox->setVisible(true);
        }

        if (line.indexOf("--structured-cc-mode") != -1) {
            m_ui->pwStructuredCCModeCheckBox->setVisible(true);
            m_ui->pwStructuredCCModeComboBox->setVisible(true);
        }

        if (line.indexOf("--structured-ssn-count") != -1) {
            m_ui->pwStructuredSSNCountCheckBox->setVisible(true);
            m_ui->pwStructuredSSNCountSpinBox->setVisible(true);
        }

        if (line.indexOf("--structured-ssn-format") != -1) {
            m_ui->pwstructuredSSNFormatCheckBox->setVisible(true);
            m_ui->pwStructuredSSNFormatComboBox->setVisible(true);
        }

        if (line.indexOf("--fail-if-cvd-older-than") != -1) {
            m_ui->pwdatabaseOutdatedCheckbox->setVisible(true);
            m_ui->pwdatabaseOutdatedSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-filesize") != -1) {
            m_ui->pwfilesLargerThanThisCheckBox->setVisible(true);
            m_ui->pwfilesLargerThanThisComboBox->setVisible(true);
            m_ui->pwfilesLargerThanThisSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-scansize") != -1) {
            m_ui->pwmaxAmountForContainerCheckBox->setVisible(true);
            m_ui->pwmaxAmountForContainerComboBox->setVisible(true);
            m_ui->pwmaxAmountForContainerSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-recursion") != -1) {
            m_ui->pwmaxArchiveRecursionForContainerCheckBox->setVisible(true);
            m_ui->pwmaxArchiveRecursionForContainerComboBox->setVisible(true);
            m_ui->pwmaxArchiveRecursionForContainerSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-dir-recursion") != -1) {
            m_ui->pwmaxDirRecursionLevelCheckBox->setVisible(true);
            m_ui->pwmaxDirRecursionLevelComboBox->setVisible(true);
            m_ui->pwmaxDirRecursionLevelSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-files") != -1) {
            m_ui->pwmaxNumberForContainerCheckBox->setVisible(true);
            m_ui->pwmaxNumberForContainerComboBox->setVisible(true);
            m_ui->pwmaxNumberForContainerSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-iconspe") != -1) {
            m_ui->pwmaxNumberOfIconsInPEFileCheckBox->setVisible(true);
            m_ui->pwmaxNumberOfIconsInPEFileComboBox->setVisible(true);
            m_ui->pwmaxNumberOfIconsInPEFileSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-partitions") != -1) {
            m_ui->pwmaxNumberOfPartitionsInDiskImageCheckBox->setVisible(true);
            m_ui->pwmaxNumberOfPartitionsInDiskImageComboBox->setVisible(true);
            m_ui->pwmaxNumberOfPartitionsInDiskImageSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-embeddedpe") != -1) {
            m_ui->pwmaxSizeFileForPECheckBox->setVisible(true);
            m_ui->pwmaxSizeFileForPEComboBox->setVisible(true);
            m_ui->pwmaxSizeFileForPESpinBox->setVisible(true);
        }

        if (line.indexOf("--max-htmlnormalize") != -1) {
            m_ui->pwmaxSizeHTMLFileToNormalizeCheckBox->setVisible(true);
            m_ui->pwmaxSizeHTMLFileToNormalizeComboBox->setVisible(true);
            m_ui->pwmaxSizeHTMLFileToNormalizeSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-htmlnotags") != -1) {
            m_ui->pwmaxSizeOfNormalizedHTMLFileCheckBox->setVisible(true);
            m_ui->pwmaxSizeOfNormalizedHTMLFileComboBox->setVisible(true);
            m_ui->pwmaxSizeOfNormalizedHTMLFileSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-scriptnormalize") != -1) {
            m_ui->pwmaxSizeOfScriptFileToNormalizeCheckBox->setVisible(true);
            m_ui->pwmaxSizeOfScriptFileToNormalizeComboBox->setVisible(true);
            m_ui->pwmaxSizeOfScriptFileToNormalizeSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-ziptypercg") != -1) {
            m_ui->pwmaxSizeZipToTypeReanalzeCheckBox->setVisible(true);
            m_ui->pwmaxSizeZipToTypeReanalzeComboBox->setVisible(true);
            m_ui->pwmaxSizeZipToTypeReanalzeSpinBox->setVisible(true);
        }

        if (line.indexOf("--stats-timeout") != -1) {
            m_ui->pwnumberOfSecondsForResponseCheckBox->setVisible(true);
            m_ui->pwnumberOfSecondsForResponseSpinBox->setVisible(true);
            m_ui->limitFrame_13->setVisible(true);
        }
    }
}

void ProfileWizardDialog::slot_cancleButtonClicked()
{
    QString profileFile = QDir::homePath() + "/.clamav-gui/profiles/" + m_ui->profileNameLineEdit->text() + ".ini";
    if ((m_newProfile == true) && (profileFile != QDir::homePath() + "/.clamav-gui/settings.ini")) {
        QFile killfile(profileFile);
        if (killfile.exists() == true)
            killfile.remove();
    }
    this->close();
}

void ProfileWizardDialog::slot_hiddenDirsCheckBoxClicked()
{
    m_ui->showHiddenDirsCheckBox->isChecked() == true ? m_model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden)
                                                      : m_model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
}

void ProfileWizardDialog::slot_directoryCheckBoxesClicked()
{
    if (m_ui->loadVirusDatabaseCheckBox->isChecked() == true) {
        m_ui->lvdFrame->setStyleSheet(css);
        m_ui->loadVirusDatabaseLineEdit->setEnabled(true);
        m_ui->pwselectLVDButton->setEnabled(true);
    }
    else {
        m_ui->lvdFrame->setStyleSheet("");
        m_ui->loadVirusDatabaseLineEdit->setEnabled(false);
        m_ui->pwselectLVDButton->setEnabled(false);
    }

    if (m_ui->scanReportToFileCheckBox->isChecked() == true) {
        m_ui->ssrtfFrame->setStyleSheet(css);
        m_ui->scanReportToFileLineEdit->setEnabled(true);
        m_ui->pwselectSCRFButton->setEnabled(true);
    }
    else {
        m_ui->ssrtfFrame->setStyleSheet("");
        m_ui->scanReportToFileLineEdit->setEnabled(false);
        m_ui->pwselectSCRFButton->setEnabled(false);
    }

    if (m_ui->scanFilesFromFileCheckBox->isChecked() == true) {
        m_ui->sfffFrame->setStyleSheet(css);
        m_ui->scanFilesFromFileLineEdit->setEnabled(true);
        m_ui->pwselectSFFButton->setEnabled(true);
    }
    else {
        m_ui->sfffFrame->setStyleSheet("");
        m_ui->scanFilesFromFileLineEdit->setEnabled(false);
        m_ui->pwselectSFFButton->setEnabled(false);
    }

    if (m_ui->tempFileCheckBox->isEnabled() == true) {
        m_ui->ctfidFrame->setStyleSheet(css);
        m_ui->tempFilesLineEdit->setEnabled(true);
        m_ui->pwselectTFButton->setEnabled(true);
    }
    else {
        m_ui->ctfidFrame->setStyleSheet("");
        m_ui->tempFilesLineEdit->setEnabled(false);
        m_ui->pwselectTFButton->setEnabled(false);
    }

    if (m_ui->moveDirectoryCheckBox->isChecked() == true) {
        m_ui->mifidFrame->setStyleSheet(css);
        m_ui->moveDirectoryLineEdit->setEnabled(true);
        m_ui->pwselectMDButton->setEnabled(true);
    }
    else {
        m_ui->mifidFrame->setStyleSheet("");
        m_ui->moveDirectoryLineEdit->setEnabled(false);
        m_ui->pwselectMDButton->setEnabled(false);
    }

    if (m_ui->copyDirectoryCheckBox->isChecked() == true) {
        m_ui->cifidFrame->setStyleSheet(css);
        m_ui->copyDirectoryLineEdit->setEnabled(true);
        m_ui->pwselectCFButton->setEnabled(true);
    }
    else {
        m_ui->cifidFrame->setStyleSheet("");
        m_ui->copyDirectoryLineEdit->setEnabled(false);
        m_ui->pwselectCFButton->setEnabled(false);
    }

    if (m_ui->followDirectorySymlinksCheckBox->isChecked() == true) {
        m_ui->fdsFrame->setStyleSheet(css);
        m_ui->followDirectorySymlinksComboBox->setEnabled(true);
    }
    else {
        m_ui->fdsFrame->setStyleSheet("");
        m_ui->followDirectorySymlinksComboBox->setEnabled(false);
    }

    if (m_ui->followFileSymlinksCheckBox->isChecked() == true) {
        m_ui->ffsFrame->setStyleSheet(css);
        m_ui->followFileSymlinksComboBox->setEnabled(true);
    }
    else {
        m_ui->ffsFrame->setStyleSheet("");
        m_ui->followFileSymlinksComboBox->setEnabled(false);
    }
}

void ProfileWizardDialog::slot_scanLimitsCheckBoxClicked()
{
    if (m_ui->pwfilesLargerThanThisCheckBox->isChecked()) {
        m_ui->limitFrame_1->setStyleSheet(css);
        m_ui->pwfilesLargerThanThisComboBox->setEnabled(true);
        m_ui->pwfilesLargerThanThisSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_1->setStyleSheet("");
        m_ui->pwfilesLargerThanThisComboBox->setEnabled(false);
        m_ui->pwfilesLargerThanThisSpinBox->setEnabled(false);
    }

    if (m_ui->pwmaxAmountForContainerCheckBox->isChecked()) {
        m_ui->limitFrame_2->setStyleSheet(css);
        m_ui->pwmaxAmountForContainerComboBox->setEnabled(true);
        m_ui->pwmaxAmountForContainerSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_2->setStyleSheet("");
        m_ui->pwmaxAmountForContainerComboBox->setEnabled(false);
        m_ui->pwmaxAmountForContainerSpinBox->setEnabled(false);
    }

    if (m_ui->pwmaxNumberForContainerCheckBox->isChecked()) {
        m_ui->limitFrame_3->setStyleSheet(css);
        m_ui->pwmaxNumberForContainerComboBox->setEnabled(true);
        m_ui->pwmaxNumberForContainerSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_3->setStyleSheet("");
        m_ui->pwmaxNumberForContainerComboBox->setEnabled(false);
        m_ui->pwmaxNumberForContainerSpinBox->setEnabled(false);
    }

    if (m_ui->pwmaxArchiveRecursionForContainerCheckBox->isChecked()) {
        m_ui->limitFrame_4->setStyleSheet(css);
        m_ui->pwmaxArchiveRecursionForContainerComboBox->setEnabled(true);
        m_ui->pwmaxArchiveRecursionForContainerSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_4->setStyleSheet("");
        m_ui->pwmaxArchiveRecursionForContainerComboBox->setEnabled(false);
        m_ui->pwmaxArchiveRecursionForContainerSpinBox->setEnabled(false);
    }

    if (m_ui->pwmaxDirRecursionLevelCheckBox->isChecked()) {
        m_ui->limitFrame_5->setStyleSheet(css);
        m_ui->pwmaxDirRecursionLevelComboBox->setEnabled(true);
        m_ui->pwmaxDirRecursionLevelSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_5->setStyleSheet("");
        m_ui->pwmaxDirRecursionLevelComboBox->setEnabled(false);
        m_ui->pwmaxDirRecursionLevelSpinBox->setEnabled(false);
    }

    if (m_ui->pwmaxSizeFileForPECheckBox->isChecked()) {
        m_ui->limitFrame_6->setStyleSheet(css);
        m_ui->pwmaxSizeFileForPEComboBox->setEnabled(true);
        m_ui->pwmaxSizeFileForPESpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_6->setStyleSheet("");
        m_ui->pwmaxSizeFileForPEComboBox->setEnabled(false);
        m_ui->pwmaxSizeFileForPESpinBox->setEnabled(false);
    }

    if (m_ui->pwmaxSizeHTMLFileToNormalizeCheckBox->isChecked()) {
        m_ui->limitFrame_7->setStyleSheet(css);
        m_ui->pwmaxSizeHTMLFileToNormalizeComboBox->setEnabled(true);
        m_ui->pwmaxSizeHTMLFileToNormalizeSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_7->setStyleSheet("");
        m_ui->pwmaxSizeHTMLFileToNormalizeComboBox->setEnabled(false);
        m_ui->pwmaxSizeHTMLFileToNormalizeSpinBox->setEnabled(false);
    }

    if (m_ui->pwmaxSizeOfNormalizedHTMLFileCheckBox->isChecked()) {
        m_ui->limitFrame_8->setStyleSheet(css);
        m_ui->pwmaxSizeOfNormalizedHTMLFileComboBox->setEnabled(true);
        m_ui->pwmaxSizeOfNormalizedHTMLFileSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_8->setStyleSheet("");
        m_ui->pwmaxSizeOfNormalizedHTMLFileComboBox->setEnabled(false);
        m_ui->pwmaxSizeOfNormalizedHTMLFileSpinBox->setEnabled(false);
    }

    if (m_ui->pwmaxSizeOfScriptFileToNormalizeCheckBox->isChecked()) {
        m_ui->limitFrame_9->setStyleSheet(css);
        m_ui->pwmaxSizeOfScriptFileToNormalizeComboBox->setEnabled(true);
        m_ui->pwmaxSizeOfScriptFileToNormalizeSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_9->setStyleSheet("");
        m_ui->pwmaxSizeOfScriptFileToNormalizeComboBox->setEnabled(false);
        m_ui->pwmaxSizeOfScriptFileToNormalizeSpinBox->setEnabled(false);
    }

    if (m_ui->pwmaxSizeZipToTypeReanalzeCheckBox->isChecked()) {
        m_ui->limitFrame_10->setStyleSheet(css);
        m_ui->pwmaxSizeZipToTypeReanalzeComboBox->setEnabled(true);
        m_ui->pwmaxSizeZipToTypeReanalzeSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_10->setStyleSheet("");
        m_ui->pwmaxSizeZipToTypeReanalzeComboBox->setEnabled(false);
        m_ui->pwmaxSizeZipToTypeReanalzeSpinBox->setEnabled(false);
    }

    if (m_ui->pwmaxNumberOfPartitionsInDiskImageCheckBox->isChecked()) {
        m_ui->limitFrame_11->setStyleSheet(css);
        m_ui->pwmaxNumberOfPartitionsInDiskImageComboBox->setEnabled(true);
        m_ui->pwmaxNumberOfPartitionsInDiskImageSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_11->setStyleSheet("");
        m_ui->pwmaxNumberOfPartitionsInDiskImageComboBox->setEnabled(false);
        m_ui->pwmaxNumberOfPartitionsInDiskImageSpinBox->setEnabled(false);
    }

    if (m_ui->pwmaxNumberOfIconsInPEFileCheckBox->isChecked()) {
        m_ui->limitFrame_12->setStyleSheet(css);
        m_ui->pwmaxNumberOfIconsInPEFileComboBox->setEnabled(true);
        m_ui->pwmaxNumberOfIconsInPEFileSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_12->setStyleSheet("");
        m_ui->pwmaxNumberOfIconsInPEFileComboBox->setEnabled(false);
        m_ui->pwmaxNumberOfIconsInPEFileSpinBox->setEnabled(false);
    }

    if (m_ui->pwnumberOfSecondsForResponseCheckBox->isChecked()) {
        m_ui->limitFrame_13->setStyleSheet(css);
        m_ui->pwnumberOfSecondsForResponseSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_13->setStyleSheet("");
        m_ui->pwnumberOfSecondsForResponseSpinBox->setEnabled(false);
    }

    if (m_ui->pwBytecodeTimeoutCheckBox->isChecked()) {
        m_ui->limitFrame_14->setStyleSheet(css);
        m_ui->pwBytecodeTimeoutSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_14->setStyleSheet("");
        m_ui->pwBytecodeTimeoutSpinBox->setEnabled(false);
    }

    if (m_ui->pwExecutionStatisticsCheckBox->isChecked()) {
        m_ui->limitFrame_15->setStyleSheet(css);
        m_ui->pwExecutionStatisticsComboBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_15->setStyleSheet("");
        m_ui->pwExecutionStatisticsComboBox->setEnabled(false);
    }

    if (m_ui->pwstructuredSSNFormatCheckBox->isChecked()) {
        m_ui->limitFrame_16->setStyleSheet(css);
        m_ui->pwStructuredSSNFormatComboBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_16->setStyleSheet("");
        m_ui->pwStructuredSSNFormatComboBox->setEnabled(false);
    }

    if (m_ui->pwStructuredSSNCountCheckBox->isChecked()) {
        m_ui->limitFrame_17->setStyleSheet(css);
        m_ui->pwStructuredSSNCountSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_17->setStyleSheet("");
        m_ui->pwStructuredSSNCountSpinBox->setEnabled(false);
    }

    if (m_ui->pwStructuredCCCountCheckBox->isChecked()) {
        m_ui->limitFrame_18->setStyleSheet(css);
        m_ui->pwStructuredCCCountSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_18->setStyleSheet("");
        m_ui->pwStructuredCCCountSpinBox->setEnabled(false);
    }

    if (m_ui->pwStructuredCCModeCheckBox->isChecked()) {
        m_ui->limitFrame_19->setStyleSheet(css);
        m_ui->pwStructuredCCModeComboBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_19->setStyleSheet("");
        m_ui->pwStructuredCCModeComboBox->setEnabled(false);
    }

    if (m_ui->pwMaxScanTimeCheckBox->isChecked()) {
        m_ui->limitFrame_20->setStyleSheet(css);
        m_ui->pwMaxScanTimeSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_20->setStyleSheet("");
        m_ui->pwMaxScanTimeSpinBox->setEnabled(false);
    }

    if (m_ui->pwMaxRecursionHWP3CheckBox->isChecked()) {
        m_ui->limitFrame_21->setStyleSheet(css);
        m_ui->pwMaxRecursionHWP3SpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_21->setStyleSheet("");
        m_ui->pwMaxRecursionHWP3SpinBox->setEnabled(false);
    }

    if (m_ui->pwMaxCallsPCREMatchFunctionCheckBox->isChecked()) {
        m_ui->limitFrame_22->setStyleSheet(css);
        m_ui->pwMaxCallsPCREMatchFunctionSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_22->setStyleSheet("");
        m_ui->pwMaxCallsPCREMatchFunctionSpinBox->setEnabled(false);
    }

    if (m_ui->pwMaxRecursionCallsPCREMatchFunctionCheckBox->isChecked()) {
        m_ui->limitFrame_23->setStyleSheet(css);
        m_ui->pwMaxRecursionCallsPCREMatchFunctionCpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_23->setStyleSheet("");
        m_ui->pwMaxRecursionCallsPCREMatchFunctionCpinBox->setEnabled(false);
    }

    if (m_ui->pwMaxPCREFileSizeCheckBox->isChecked()) {
        m_ui->limitFrame_24->setStyleSheet(css);
        m_ui->pwMaxPCREFileSizeComboBox->setEnabled(true);
        m_ui->pwMaxPCREFileSizeSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_24->setStyleSheet("");
        m_ui->pwMaxPCREFileSizeComboBox->setEnabled(false);
        m_ui->pwMaxPCREFileSizeSpinBox->setEnabled(false);
    }

    if (m_ui->pwdatabaseOutdatedCheckbox->isChecked()) {
        m_ui->limitFrame_25->setStyleSheet(css);
        m_ui->pwdatabaseOutdatedSpinBox->setEnabled(true);
    }
    else {
        m_ui->limitFrame_25->setStyleSheet("");
        m_ui->pwdatabaseOutdatedSpinBox->setEnabled(false);
    }
}

void ProfileWizardDialog::slot_highlightSettings()
{

    m_ui->pwdontScanFileNameCheckBox->isChecked() ? m_ui->frame01->setStyleSheet(css) : m_ui->frame01->setStyleSheet("");
    m_ui->pwdontScanDirCheckBox->isChecked() ? m_ui->frame02->setStyleSheet(css) : m_ui->frame02->setStyleSheet("");
    m_ui->pwonlyScanFileNameCheckBox->isChecked() ? m_ui->frame03->setStyleSheet(css) : m_ui->frame03->setStyleSheet("");
    m_ui->pwonlyScanDirCheckBox->isChecked() ? m_ui->frame04->setStyleSheet(css) : m_ui->frame04->setStyleSheet("");

    m_ui->pwloadPUAPackedRadioButon->isChecked() ? m_ui->pwloadPUAPackedRadioButon->setStyleSheet(css)
                                                 : m_ui->pwloadPUAPackedRadioButon->setStyleSheet("");
    m_ui->pwloadPUAPWToolRadioButton->isChecked() ? m_ui->pwloadPUAPWToolRadioButton->setStyleSheet(css)
                                                  : m_ui->pwloadPUAPWToolRadioButton->setStyleSheet("");
    m_ui->pwloadPUANetToolRadioButton->isChecked() ? m_ui->pwloadPUANetToolRadioButton->setStyleSheet(css)
                                                   : m_ui->pwloadPUANetToolRadioButton->setStyleSheet("");
    m_ui->pwloadPUAP2PRadioButton->isChecked() ? m_ui->pwloadPUAP2PRadioButton->setStyleSheet(css) : m_ui->pwloadPUAP2PRadioButton->setStyleSheet("");
    m_ui->pwloadPUAIRCRadioButton->isChecked() ? m_ui->pwloadPUAIRCRadioButton->setStyleSheet(css) : m_ui->pwloadPUAIRCRadioButton->setStyleSheet("");
    m_ui->pwloadPUARATRadioButton->isChecked() ? m_ui->pwloadPUARATRadioButton->setStyleSheet(css) : m_ui->pwloadPUARATRadioButton->setStyleSheet("");
    m_ui->pwloadPUANetToolSpyRadioButton->isChecked() ? m_ui->pwloadPUANetToolSpyRadioButton->setStyleSheet(css)
                                                      : m_ui->pwloadPUANetToolSpyRadioButton->setStyleSheet("");
    m_ui->pwloadPUAServerRadioButton->isChecked() ? m_ui->pwloadPUAServerRadioButton->setStyleSheet(css)
                                                  : m_ui->pwloadPUAServerRadioButton->setStyleSheet("");
    m_ui->pwloadPUAScriptRadioButton->isChecked() ? m_ui->pwloadPUAScriptRadioButton->setStyleSheet(css)
                                                  : m_ui->pwloadPUAScriptRadioButton->setStyleSheet("");
    m_ui->pwloadPUAAndrRadioButton->isChecked() ? m_ui->pwloadPUAAndrRadioButton->setStyleSheet(css)
                                                : m_ui->pwloadPUAAndrRadioButton->setStyleSheet("");
    m_ui->pwloadPUAJavaRadioButton->isChecked() ? m_ui->pwloadPUAJavaRadioButton->setStyleSheet(css)
                                                : m_ui->pwloadPUAJavaRadioButton->setStyleSheet("");
    m_ui->pwloadPUAOsxRadioButton->isChecked() ? m_ui->pwloadPUAOsxRadioButton->setStyleSheet(css) : m_ui->pwloadPUAOsxRadioButton->setStyleSheet("");
    m_ui->pwloadPUAToolRadioButton->isChecked() ? m_ui->pwloadPUAToolRadioButton->setStyleSheet(css)
                                                : m_ui->pwloadPUAToolRadioButton->setStyleSheet("");
    m_ui->pwloadPUAUnixRadioButton->isChecked() ? m_ui->pwloadPUAUnixRadioButton->setStyleSheet(css)
                                                : m_ui->pwloadPUAUnixRadioButton->setStyleSheet("");
    m_ui->pwloadPUAWinRadioButton->isChecked() ? m_ui->pwloadPUAWinRadioButton->setStyleSheet(css) : m_ui->pwloadPUAWinRadioButton->setStyleSheet("");
}
