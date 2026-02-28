#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QWidget>
#include "setupfilehandler.h"
#include "profilewizarddialog.h"

namespace Ui {
class ProfileManager;
}

class ProfileManager : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileManager(QWidget *parent = 0, setupFileHandler *setupFile = 0);
    ~ProfileManager();

private:
    setupFileHandler    *m_setupFile;
    Ui::ProfileManager  *m_ui;
    ProfileWizardDialog *m_profileWizard;
    
    void getProfileList();
    void readProfileSettings();
    void checkMonochromeSettings();

private slots:
    void slot_readProfileSettings();
    void slot_addProfileButtonClicked();
    void slot_eraseProfileButtonClicked();
    void slot_editProfileButtonClicked();
    void slot_profileSaved();
    void monochromeModeChanged(bool state);

signals:
    void triggerProfilesChanged();
};

#endif // PROFILEMANAGER_H
