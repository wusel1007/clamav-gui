#ifndef SCANOPTIONYN_H
#define SCANOPTIONYN_H

#include <QWidget>
#include "ui_scanoptionyn.h"
#include "scanoptionbaseclass.h"
#include "setupfilehandler.h"
#include "toolbox.h"

namespace Ui {
class scanoptionyn;
}

class scanoptionyn : public scanOptionBaseClass
{
    Q_OBJECT

public:
    explicit scanoptionyn(QWidget *parent = nullptr, QString setupFileName = "", QString section = "", bool checked = false, QString label = "", QString comment = "");
    ~scanoptionyn() = default;
    QString getOption();
    QString getComment();
    bool isChecked();
    void setCurrentText(QString text);

private:
    Ui::scanoptionyn m_ui;
    setupFileHandler * m_setupFile;
    QString m_setupFileSection;
    QString m_option;
    QString m_com;

private slots:
    void slot_checkboxClicked();
    void slot_comboboxChanged(QString value);

signals:
    void valuechanged();
};

#endif // SCANOPTIONYN_H
