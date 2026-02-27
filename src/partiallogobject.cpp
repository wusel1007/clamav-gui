#include "partiallogobject.h"
#include "ui_partiallogobject.h"

partialLogObject::partialLogObject(QWidget* parent, QString logText, bool highlighterDisabled) 
: QWidget(parent)
{
    m_ui.setupUi(this);
    m_logHighlighter = NULL;
    if (highlighterDisabled == false) {
        m_logHighlighter = new highlighter(m_ui.logPlainText->document());
    }
    setLogText(logText);
}

void partialLogObject::setLogText(QString logText) {
    QString engine;
    QString scannedDirs;
    QString scannedFiles;
    QString errors;
    QString infectedFiles;
    int pos;

    m_ui.logPlainText->setPlainText(logText);

    pos = logText.indexOf("Engine version:");
    if (pos > -1) {
        engine = logText.mid(pos + 15, logText.indexOf("\n", pos + 15) - (pos + 15));
    } else {
        engine = "n/a";
    }

    pos = logText.indexOf("Scanned directories:");
    if (pos > -1) {
        scannedDirs = logText.mid(pos + 20, logText.indexOf("\n", pos + 20) - (pos + 20));
    } else {
        scannedDirs = "n/a";
    }

    pos = logText.indexOf("Scanned files:");
    if (pos > -1) {
        scannedFiles = logText.mid(pos + 14, logText.indexOf("\n", pos + 14) - (pos + 14));
    } else {
        scannedFiles = "n/a";
    }

    pos = logText.indexOf("Total errors:");
    if (pos > -1) {
        errors = logText.mid(pos + 13, logText.indexOf("\n", pos + 13) - (pos + 13));
    } else {
        errors = "n/a";
    }

    pos = logText.indexOf("Infected files:");
    if (pos > -1) {
        infectedFiles = logText.mid(pos + 15, logText.indexOf("\n", pos + 15) - (pos + 15));
    } else {
        infectedFiles = "n/a";
    }

    m_ui.engineVersionLabel->setText(tr("Engine Version: ") + engine);
    m_ui.scannedDirectoriesLabel->setText(tr("Scanned Directories: ") + scannedDirs);
    m_ui.scannedFilesLabel->setText(tr("Scanned Files: ") + scannedFiles);
    m_ui.totalErrorsLabel->setText(tr("Total Errors: ") + errors);
    m_ui.infectedFilesLabel->setText(tr("Infected Files: ") + infectedFiles);
}

QString partialLogObject::getLogText() {
    return "<Scanning startet> " + m_ui.logPlainText->toPlainText();
}

void partialLogObject::slot_searchButtonClicked() {
    QTextCursor cursor = m_ui.logPlainText->textCursor();
    QString searchString = m_ui.searchTextLineEdit->text();
    int pos = m_ui.logPlainText->toPlainText().toUpper().indexOf(searchString.toUpper(), m_start);

    if (pos > -1) {
        m_ui.searchButton->setText(tr("continue"));
        m_start = pos + searchString.length();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, searchString.length());
        m_ui.logPlainText->setTextCursor(cursor);
        m_ui.logPlainText->ensureCursorVisible();
    } else {
        if (QMessageBox::question(this, "INFO", tr("Searchstring not found!\nContinue from the Start of the Log?"), QMessageBox::Yes,
                                  QMessageBox::No) == QMessageBox::Yes) {
            m_start = 0;
            pos = m_ui.logPlainText->toPlainText().toUpper().indexOf(searchString.toUpper(), m_start);
            if (pos > -1) {
                m_ui.searchButton->setText(tr("continue"));
                m_start = pos + searchString.length();
                cursor.movePosition(QTextCursor::Start);
                cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos);
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, searchString.length());
                m_ui.logPlainText->setTextCursor(cursor);
                m_ui.logPlainText->ensureCursorVisible();
            } else {
                QMessageBox::information(this, tr("INFO"), tr("Searchstring not found!"));
            }
        }
    }
}

void partialLogObject::slot_searchLineEditChanged() {
    m_start = 0;
    m_ui.searchButton->setText(tr("search"));
}

void partialLogObject::slot_clearLineEditButtonClicked() {
    m_ui.searchTextLineEdit->setText("");
    m_start = 0;
}

void partialLogObject::slot_totalErrorButtonClicked() {
    QTextCursor cursor = m_ui.logPlainText->textCursor();
    QStringList searchStrings;
    QString searchString;
    int pos = -1;
    int index = 0;

    searchStrings << "Access denied" << "Empty file";

    while ((pos == -1) & (index < searchStrings.count())) {
        searchString = searchStrings.at(index);
        index++;
        pos = m_ui.logPlainText->toPlainText().indexOf(searchString, m_errorStart);
    }

    if (pos > -1) {
        m_errorStart = pos + searchString.length();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, searchString.length());
        m_ui.logPlainText->setTextCursor(cursor);
        m_ui.logPlainText->ensureCursorVisible();
        if (m_ui.logPlainText->toPlainText().indexOf(searchString, m_errorStart) == -1)
            m_errorStart = 0;
    }
}

void partialLogObject::slot_infectedFilesButtonClicked() {
    QTextCursor cursor = m_ui.logPlainText->textCursor();
    QString searchString = " FOUND\n";
    int pos = m_ui.logPlainText->toPlainText().indexOf(searchString, m_infectedStart);

    if (pos > -1) {
        m_infectedStart = pos + searchString.length();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, searchString.length());
        m_ui.logPlainText->setTextCursor(cursor);
        m_ui.logPlainText->ensureCursorVisible();
        if (m_ui.logPlainText->toPlainText().indexOf(searchString, m_infectedStart) == -1)
            m_infectedStart = 0;
    }
}

void partialLogObject::slot_add_remove_highlighter(bool state) {
    if (state == true) {
        if (m_logHighlighter != NULL) {
            delete m_logHighlighter;
            m_logHighlighter = NULL;
        }
    } else {
        if (m_logHighlighter == NULL) {
            m_logHighlighter = new highlighter(m_ui.logPlainText->document());
        } else {
            delete m_logHighlighter;
            m_logHighlighter = new highlighter(m_ui.logPlainText->document());
        }
    }
}
