#ifndef CLAMDMANAGER_H
#define CLAMDMANAGER_H

#include "clamdconfcomboboxoption.h"
#include "clamdconfspinboxoption.h"
#include "clamdconfstringoption.h"
#include "clamdconfmultioption.h"
#include "clamdconfoptionbaseclass.h"
#include "setupfilehandler.h"
#include "ui_clamdmanager.h"
#include "highlighter.h"
#include "translator.h"
#include "toolbox.h"

#include <QFileSystemWatcher>
#include <QFileDialog>
#include <QTextStream>
#include <QProcess>
#include <QWidget>
#include <QMovie>
#include <QList>
#include <QFile>
#include <QTimer>

namespace Ui
{
class clamdManager;
}

class clamdManager : public QWidget
{
    Q_OBJECT

public:
    explicit clamdManager(QWidget* parent = 0, setupFileHandler* setupFile = 0);
    ~clamdManager() = default;

private:
    Ui::clamdManager m_ui;
    setupFileHandler* m_setupFile;  // clamd && freshclam
    setupFileHandler* m_clamdConf;  // clamd
    setupFileHandler* m_freshclamConf;
    QProcess* m_startClamdProcess;         // clamd
    QProcess* m_findclamonaccProcess;      // clamd
    QProcess* m_restartClamonaccProcess;   // clamd
    QProcess* m_killProcess;
    QProcess* m_getClamdConfParametersProcess;
    QProcess* m_findClamdProcess;
    QString m_clamdLocation;      // clamd
    QString m_clamonaccLocation;  // clamd
    QString m_clamonaccPid;       // clamd
    QString m_lastFound;          // clamd
    QString m_sudoGUI;
    QTimer* m_startDelayTimer;
    QTimer* m_processWatcher;
    highlighter* m_logHighlighter;          // clamd
    QList <ClamdConfOptionBaseClass*> m_clamdConfParameters;   // clamd
    QFileSystemWatcher* m_clamdLogWatcher;  // clamd
    QFileSystemWatcher* m_clamdPidWatcher;  // clamd
    int m_dirsUnderMonitoring;
    bool m_clamdRestartInProgress;  // clamd
    bool m_monochrome;
    bool m_startup;
    bool m_waitForFreshclam;
    bool m_initprocessrunning;
    bool m_clamdManagerLocked;
    int m_clamdStartupCounter;

    QString trimLocationOutput(QString);
    void restartClamonacc();   // clamd
    bool checkClamdRunning();  // clamd
    void getClamdConfElements();
    QString selectColor(QString color);

private slots:
    void slot_updateClamdConf();  // ab hier --> clamd
    void slot_logFileContentChanged();
    void slot_clamdStartStopButtonClicked();
    void slot_pidWatcherTriggered();
    void slot_startClamdProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void slot_killClamdProcessFinished();
    void slot_findclamonaccProcessFinished(int rc);
    void slot_monitoringAddButtonClicked();
    void slot_monitoringDelButtonClicked();
    void slot_restartClamonaccProcessFinished();
    void slot_restartClamdButtonClicked();
    void slot_clamdSettingsChanged();
    void slot_startClamdOnStartupCheckBoxClicked();
    void slot_startDelayTimerExpired();
    void slot_waitForFreshclamStarted();
    void slot_processWatcherExpired();
    void slot_add_remove_highlighter(bool);
    void slot_initClamdSettings();  // clamd
    void slot_dbPathChanged(QString dbPath);
    void slot_updateClamdConfParameters();
    void slot_getClamdConfParameterProcessFinished();
    void slot_filterChanged(QString);
    void slot_showSelectedChecked();
    void slot_showUnselectedChecked();
    void slot_findClamdProcessFinished();


signals:
    void setBallonMessage(int, QString, QString);  // clamd
    void setActiveTab();
    void systemStatusChanged();
};

#endif  // CLAMDMANAGER_H
