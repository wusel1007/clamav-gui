#include "schedulescanobject.h"

scheduleScanObject::scheduleScanObject(QWidget* parent, QString name, QStringList parameters) : QDialog(parent), scanJob(name)
{
    bool useclamdscan = false;

    setWindowFlags(((this->windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint & ~Qt::WindowContextHelpButtonHint));
    m_ui.setupUi(this);

    if (name == "Direct Scan") {
        m_directScan = true;
        m_ui.headerLabel->setText("Direct Scan");
    }
    else {
        m_directScan = false;
    }

    m_setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini", this);

    m_logHighLighter = new highlighter(m_ui.logMessagePlainTextEdit->document());

    m_ui.scanJobHeader->setText(tr("Scan-Job: ") + name);
    this->setWindowTitle(tr("Scheduled Scan-Job: ") + name);

    m_scanProcess = new QProcess(this);
    connect(m_scanProcess, SIGNAL(readyReadStandardError()), this, SLOT(slot_scanProcessHasErrOutput()));
    connect(m_scanProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(slot_scanProcessHasStdOutput()));
    connect(m_scanProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slot_scanProcessFinished(int, QProcess::ExitStatus)));

    QString message;

    if (m_setupFile->getSectionValue("Clamd", "Status") == "is running") {
        // TODO: use enumerate
        switch (m_setupFile->getSectionIntValue("Clamd", "ClamdScanMultithreading")) {
            case 0:
                useclamdscan = false;
                message = "clamscan ";
                break;
            case 1:
                useclamdscan = true;
                message = "clamdscan ";
                break;
            case 2:
                if (m_directScan == true) {
                    useclamdscan = false;
                    message = "clamscan ";
                }
                else {
                    useclamdscan = true;
                    message = "clamdscan ";
                }
                break;
            case 3:
                if (m_directScan == false) {
                    useclamdscan = false;
                    message = "clamdscan ";
                }
                else {
                    useclamdscan = true;
                    message = "clamdscan ";
                }
                break;
            case 4:
                if (QMessageBox::question(this, tr("Use ClamdScan"), tr("Perform scanning using clamdscan instead of clamscan?"), QMessageBox::Yes,
                                          QMessageBox::No) == QMessageBox::Yes) {
                    useclamdscan = true;
                    message = "clamdscan ";
                }
                else {
                    useclamdscan = false;
                    message = "clamscan ";
                }
        }
    }
    else {
        useclamdscan = false;
        message = "clamscan ";
    }

    if (useclamdscan == false) {
        for (int i = 0; i < parameters.count(); i++) {
            message = message + " " + parameters.at(i);
        }
        message = message + "\n";
    }
    else {
        message = message + "--multiscan --fdpass --config-file " + QDir::homePath() + "/.clamav-gui/clamd.conf";
        for (int i = 0; i < parameters.count(); i++) {
            QString para = parameters.at(i);
            if (para.indexOf("-") != 0)
                message = message + " " + parameters.at(i);
        }
        message = message + "\n";
    }

    m_ui.logMessagePlainTextEdit->appendPlainText(message);

    if (useclamdscan == true) {
        QStringList newParameters;
        newParameters << "--multiscan" << "--fdpass" << "--config-file" << QDir::homePath() + "/.clamav-gui/clamd.conf";
        foreach (const QString element, parameters) {
            newParameters << element;
        }
        m_scanProcess->start("clamdscan", newParameters);
    }
    else {
        m_scanProcess->start("clamscan", parameters);
    }

    m_ui.currentFileLabel->setText(tr("Scanning started ......."));

    m_busyLabel = new QLabel(this);
    m_busyLabel->setStyleSheet("background:transparent");
    m_busyLabel->setGeometry((this->width() - 80) / 2, (this->height() - 80) / 2, 80, 80);
    m_movie = new QMovie(":/icons/icons/gifs/spinning_segments_large.gif");
    m_busyLabel->setMovie(m_movie);
    m_busyLabel->show();
    m_movie->start();

    m_closeWindowTimer = new QTimer(this);
    m_closeWindowTimer->setSingleShot(true);
    connect(m_closeWindowTimer, SIGNAL(timeout()), this, SLOT(slot_closeWindowTimerTimeout()));

    m_closeWindowCounter = 100;
    m_errorStart = 0;
    m_infectedStart = 0;
}

void scheduleScanObject::accept()
{
    if (m_directScan == false)
        emit closeRequest(this);
    else
        this->done(0);
}

void scheduleScanObject::closeEvent(QCloseEvent* event)
{
    this->accept();
    event->ignore();
}

void scheduleScanObject::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        event->ignore();
    else
        event->accept();
}

void scheduleScanObject::slot_closeButtonClicked()
{
    this->accept();
}

void scheduleScanObject::slot_stopButtonClicked()
{
    if (m_scanProcess->state() == QProcess::Running) {
        m_scanProcess->kill();
    }
    m_ui.closeButton->setEnabled(true);
    m_movie->stop();
}

void scheduleScanObject::slot_scanProcessHasStdOutput()
{
    QString message = m_scanProcess->readAllStandardOutput();
    QString currentFile;
    int start, end;

    while (message.indexOf("Scanning") != -1) {
        start = message.indexOf("Scanning");
        end = message.indexOf("\n", start);
        currentFile = message.mid(start, end - start + 1);
        message.replace(currentFile, "");
        currentFile = currentFile.mid(currentFile.lastIndexOf("/") + 1);
        currentFile.replace("\n", "");
        m_ui.currentFileLabel->setText(tr("Scanning : ") + currentFile);
    }

    m_ui.logMessagePlainTextEdit->insertPlainText(message);
    m_ui.logMessagePlainTextEdit->ensureCursorVisible();
}

void scheduleScanObject::slot_scanProcessHasErrOutput()
{
    QString message = m_scanProcess->readAllStandardOutput();
    QString currentFile;
    int start, end;

    while (message.indexOf("Scanning") != -1) {
        start = message.indexOf("Scanning");
        end = message.indexOf("\n", start);
        currentFile = message.mid(start, end - start + 1);
        message.replace(currentFile, "");
        currentFile = currentFile.mid(currentFile.lastIndexOf("/") + 1);
        currentFile.replace("\n", "");
        m_ui.currentFileLabel->setText(tr("Scanning : ") + currentFile);
    }

    m_ui.logMessagePlainTextEdit->insertPlainText(message);
    m_ui.logMessagePlainTextEdit->ensureCursorVisible();
}

void scheduleScanObject::slot_scanProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    exitCode = exitCode;
    status = status;
    QString temp;
    int pos, end;

    if (m_movie != 0) {
        m_movie->stop();
        delete m_movie;
        delete m_busyLabel;
    }

    if (status == QProcess::CrashExit) {
        m_ui.currentFileLabel->setText(tr("Scan Process aborted ....."));
        m_ui.currentFileLabel->setStyleSheet("background-color:red");
        emit sendStatusReport(1, tr("Scan-Job: ") + scanJob, tr("Scan Process aborted ....."));
    }
    else if (exitCode == 0) {
        m_ui.currentFileLabel->setText(tr("Scan-Process finished ...... no Virus found!"));
        m_ui.currentFileLabel->setStyleSheet("background-color:green");
        emit sendStatusReport(0, "Scan-Job: " + scanJob, tr("Scan-Process finished ...... no Virus found!"));
        m_closeWindowTimer->start(1000);
        m_countDown = new QRoundProgressBar(this);
        m_countDown->setGeometry((this->width() - 80) / 2, (this->height() - 80) / 2, 80, 80);
        m_countDown->setMaximum(100);
        m_countDown->setMinimum(0);
        m_countDown->setValue(100);
        m_countDown->setBarStyle(QRoundProgressBar::StyleDonut);
        m_countDown->setFormat("");
        m_countDown->show();
    }
    else if (exitCode == 1) {
        m_ui.currentFileLabel->setText(tr("Scan-Process finished ...... a Virus was found!"));
        m_ui.currentFileLabel->setStyleSheet("background-color:red");
        emit sendStatusReport(2, "Scan-Job: " + scanJob, tr("Scan Process finished ..... a Virus was found!"));
    }
    else {
        m_ui.currentFileLabel->setText(tr("Scan-Process finished ...... an Error occurred!"));
        m_ui.currentFileLabel->setStyleSheet("background-color:red");
        emit sendStatusReport(1, "Scan-Job: " + scanJob, tr("Scan Process finished ..... an Error occurred!"));
    }

    m_ui.closeButton->setEnabled(true);
    m_ui.stopButton->setEnabled(false);

    if (status != QProcess::CrashExit) {
        temp = m_ui.logMessagePlainTextEdit->toPlainText();

        pos = temp.indexOf("Engine version:");
        if (pos != -1) {
            end = temp.indexOf("\n", pos);
            m_ui.engineVersionLabel->setText(tr("Engine Version: ") + temp.mid(pos + 15, end - pos - 15));
        }
        else {
            m_ui.engineVersionLabel->setText(tr("Engine Version: n/a"));
        }

        pos = temp.indexOf("Infected files:");
        if (pos != -1) {
            end = temp.indexOf("\n", pos);
            m_ui.infectedFilesLabel->setText(tr("Infected files: ") + temp.mid(pos + 15, end - pos - 15));
        }
        else {
            m_ui.infectedFilesLabel->setText(tr("Infected files: n/a"));
        }

        pos = temp.indexOf("Scanned directories:");
        if (pos != -1) {
            end = temp.indexOf("\n", pos);
            m_ui.scannedDirectoriesLabel->setText(tr("Scanned Directories: ") + temp.mid(pos + 20, end - pos - 20));
        }
        else {
            m_ui.scannedDirectoriesLabel->setText(tr("Scanned Directories: n/a"));
        }

        pos = temp.indexOf("Scanned files:");
        if (pos != -1) {
            end = temp.indexOf("\n", pos);
            m_ui.scannedFilesLabel->setText(tr("Scanned Files: ") + temp.mid(pos + 14, end - pos - 14));
        }
        else {
            m_ui.scannedFilesLabel->setText(tr("Scanned Files: n/a"));
        }

        pos = temp.indexOf("Total errors::");
        if (pos != -1) {
            end = temp.indexOf("\n", pos);
            m_ui.errorsLabel->setText(tr("Total Errors: ") + temp.mid(pos + 13, end - pos - 13));
        }
        else {
            m_ui.errorsLabel->setText(tr("Total Errors: 0"));
        }
    }

    emit scanProcessFinished();
}

void scheduleScanObject::slot_closeWindowTimerTimeout()
{
    m_closeWindowCounter--;

    if (m_closeWindowCounter == 0) {
        this->accept();
    }
    else {
        m_countDown->setValue(m_closeWindowCounter);
        m_closeWindowTimer->start(100);
    }
}

void scheduleScanObject::slot_totalErrorButtonClicked()
{
    QTextCursor cursor = m_ui.logMessagePlainTextEdit->textCursor();
    QStringList searchStrings;
    QString searchString;
    int pos = -1;
    int index = 0;

    searchStrings << "Access denied" << "Empty file";

    while ((pos == -1) & (index < searchStrings.count())) {
        searchString = searchStrings.at(index);
        index++;
        pos = m_ui.logMessagePlainTextEdit->toPlainText().indexOf(searchString, m_errorStart);
    }

    if (pos >= 0) {
        m_errorStart = pos + searchString.length();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, searchString.length());
        m_ui.logMessagePlainTextEdit->setTextCursor(cursor);
        m_ui.logMessagePlainTextEdit->ensureCursorVisible();
        if (m_ui.logMessagePlainTextEdit->toPlainText().indexOf(searchString, m_errorStart) == -1)
            m_errorStart = 0;
    }
}

void scheduleScanObject::slot_infectedFilesButtonClicked()
{
    QTextCursor cursor = m_ui.logMessagePlainTextEdit->textCursor();
    QString searchString = "FOUND";
    int pos = m_ui.logMessagePlainTextEdit->toPlainText().toUpper().indexOf(searchString, m_infectedStart);

    if (pos >= 0) {
        m_infectedStart = pos + searchString.length();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, searchString.length());
        m_ui.logMessagePlainTextEdit->setTextCursor(cursor);
        m_ui.logMessagePlainTextEdit->ensureCursorVisible();
        if (m_ui.logMessagePlainTextEdit->toPlainText().toUpper().indexOf(searchString, m_infectedStart) == -1) {
            m_infectedStart = 0;
        }
    }
}
