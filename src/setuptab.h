#ifndef SETUPTAB_H
#define SETUPTAB_H

#include <QFileSystemWatcher>
#include <QTranslator>
#include <QFileDialog>
#include <QProcess>
#include <QPixmap>
#include <QWidget>
#include <QMovie>
#include <QDir>

#include "ui_setuptab.h"
#include "setupfilehandler.h"
#include "highlighter.h"

namespace Ui {
class setupTab;
}

class setupTab : public QWidget
{
    Q_OBJECT

public:
    explicit setupTab(QWidget *parent = 0, setupFileHandler *setupFile = 0);
    ~setupTab() = default;

private:
    Ui::setupTab m_ui;
    setupFileHandler    * m_setupFile; // clamd && freshclam
    QProcess            * m_findTranslationProcess;
    bool                  m_supressMessage;
    bool                  m_monochrome;
    QString checkmonochrome(QString color);

private slots:
    void slot_selectedLanguageChanged();
    void slot_basicSettingsChanged();
    void slot_updateSystemInfo();
    void slot_clamdButtonClicked();
    void slot_clamonaccButtonClicked();
    void slot_freshclamButtonClicked();
    void slot_clamdscanComboBoxClicked();
    void slot_logHightlighterCheckBoxClicked();
    void slot_findTranslationProcessFinished();

signals:
    void switchActiveTab(int);
    void sendSystemInfo(QString);
    void logHighlightingChanged(bool);
};

#endif // SETUPTAB_H
