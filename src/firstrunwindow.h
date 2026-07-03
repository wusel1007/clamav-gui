#ifndef FIRSTRUNWINDOW_H
#define FIRSTRUNWINDOW_H

#include <QProcess>
#include <QDialog>
#include <QTimer>
#include <QDir>
#include "setupfilehandler.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class firstRunWindow;
}
QT_END_NAMESPACE

class firstRunWindow : public QDialog
{
    Q_OBJECT

public:
    firstRunWindow(QWidget *parent = nullptr);
    ~firstRunWindow();

private:
    Ui::firstRunWindow *m_ui;
    QStringList           m_initCommands;
    QStringList           m_initParameters;
    QStringList           m_processParameters;
    QProcess            * m_initProcess;
    QProcess            * m_gsettingsProcess;
    QTimer              * m_delayTimer;
    int                   m_initIndex;
    bool                  m_processInit;
    setupFileHandler    * m_setupFile;
    setupFileHandler    * m_clamdConf;
    setupFileHandler    * m_freshclamConf;
    void createBaseDirStructure();
    void createServiceMenu();
    void createInitialSettings();
    void createClamdConfFile();
    void createFreshclamConfFile();
    void findTranslation();

private slots:
    void slot_initProcessFinished();
    void slot_sddComboBoxChanged();
    void slot_languageChanged();
    void slot_done();
    void slot_findRequiredApplications();
    void slot_monochromeModeChanged();
    void slot_startupModeChanged();
    void slot_clamdscanChanged();
    void slot_gsettingsProcessFinished(int, QProcess::ExitStatus);


signals:
    void doneit();
    void settingChanged();
    void quitApplication();
};
#endif // FIRSTRUNWINDOW_H
