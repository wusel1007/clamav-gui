/***************************************************************************
 *   Copyright (C) 2015 by Joerg Zopes                                     *
 *   joerg.zopes@gmx.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include "ui_optionsdialog.h"

#include <QListWidgetItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QWidget>
#include <QList>
#include <QUrl>
#include "includeexcludeoptions.h"
#include "scanoptionbaseclass.h"
#include "setupfilehandler.h"
#include "logviewobject.h"
#include "scanlimitstab.h"
#include "scanoptionyn.h"
#include "scanoption.h"
#include "toolbox.h"

namespace Ui {
class optionsDialog;
}

class optionsDialog : public QWidget {
    Q_OBJECT

   public:
    explicit optionsDialog(QWidget* parent = 0, setupFileHandler* setupFile = 0);
    ~optionsDialog() = default;

   private:
    Ui::optionsDialog m_ui;
    setupFileHandler* m_setupFile;
    //setupFileHandler    * m_profiles;   --> obsolete, not used!
    QList <scanOptionBaseClass*> scanOptions;
    scanLimitsTab* m_scanLimits;
    includeExcludeOptions* m_incExcOptTab;
    QProcess* m_getClamscanParametersProcess;
    QString m_getClamscanProcessOutput;
    void createScanOptionElements();

    void updateDirectories();

   private slots:
    void slot_selectLVDButtonClicked();
    void slot_selectSRTFButtonClicked();
    void slot_selectMDButtonClicked();
    void slot_selectCDButtonClicked();
    void slot_selectTDButtonClicked();
    void slot_selectSFFButtonClicked();
    void writeDirectories();
    void slot_updateDirectories();
    void slot_scanReportToFileSettingsChanged();
    void slot_logViewerButtonClicked();
    void slot_getClamscanProcessHasOutput();
    void slot_getClamscanProcessFinished();
    void slot_updateClamdConf();
    void slot_scanOptionFilterChanged();
    void slot_showSelectedOnlyChecked();
    void slot_showUnselectedOnlyChecked();

   signals:
    void databasePathChanged(QString path);
    void updateDatabase();
    void updateClamdConf();
    void systemStatusChanged();
    void srtfSettingsChanged();

   public:
    QString getCopyDirectory();
    QString getMoveDirectory();
};

#endif  // OPTIONSDIALOG_H
