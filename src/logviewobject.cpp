#include "logviewobject.h"

logViewObject::logViewObject(QWidget* parent, QString fileName) : QDialog(parent)
{
    setWindowFlags(((this->windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint & ~Qt::WindowContextHelpButtonHint));
    m_ui.setupUi(this);
    m_logFileName = fileName;
    loadLogFile(fileName);
}

void logViewObject::closeEvent(QCloseEvent* event)
{
    this->accept();
    event->ignore();
}

void logViewObject::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        event->ignore();
    else
        event->accept();
}

void logViewObject::loadLogFile(QString filename)
{
    QFile file(filename);
    QString buffer;
    QStringList logs;
    QString tabHeader;
    //setupFileHandler* sf = new setupFileHandler(QDir::homePath() + "/.clamav-gm_ui/settings.ini", this);
    bool css = setupFileHandler::getSectionBoolValue(QDir::homePath() + "/.clamav-gm_ui/settings.ini","Setup", "DisableLogHighlighter");

    while (m_ui.logTab->count() > 0) {
        QWidget * tempwidget = m_ui.logTab->widget(0);
        m_ui.logTab->removeTab(0);

        if (tempwidget != nullptr) {
            delete tempwidget;
        }
    }

    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        buffer = stream.readAll();
        logs = buffer.split("<Scanning startet>");

        for (int i = 1; i < logs.count(); i++) {
            partialLogObject* log = new partialLogObject(this, logs[i], css);
            connect(this, SIGNAL(logHighlighterChanged(bool)), log, SLOT(slot_add_remove_highlighter(bool)));
            tabHeader = logs[i].mid(1, logs[i].indexOf("\n") - 1);
            m_ui.logTab->addTab(log, QIcon(":/icons/icons/information.png"), tabHeader);
        }

        file.close();
    }
}

void logViewObject::slot_closeButtonClicked()
{
    QString logText;
    partialLogObject* log;

    if (m_contentChanged) {
        if (QMessageBox::information(this, tr("INFO"), tr("Log-File was modified. Do you wanna save the changes?"), QMessageBox::Yes,
                                     QMessageBox::No) == QMessageBox::Yes) {
            if (m_ui.logTab->count() > 0) {
                for (int i = 0; i < m_ui.logTab->count(); i++) {
                    log = (partialLogObject*)m_ui.logTab->widget(i);
                    logText = logText + log->getLogText();
                    QFile logFile(m_logFileName);
                    if (logFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
                        QTextStream stream(&logFile);
                        stream << logText;
                        logFile.close();
                    }
                }
            }
            else {
                QFile logFile(m_logFileName);
                if (logFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
                    QTextStream stream(&logFile);
                    stream << "";
                    logFile.close();
                }
            }
        }
    }
    this->accept();
}

void logViewObject::slot_clearLogButtonClicked()
{
    int currentTab = m_ui.logTab->currentIndex();

    if (currentTab > -1) {
        if (QMessageBox::question(this, tr("Clear Log"), tr("Do you realy want to remove this partial log?"), QMessageBox::Yes, QMessageBox::No) ==
            QMessageBox::Yes) {
            m_ui.logTab->removeTab(currentTab);
            m_contentChanged = true;
            emit logChanged();
        }
    }
}

void logViewObject::slot_add_remove_highlighter(bool state)
{
    emit logHighlighterChanged(state);
}
