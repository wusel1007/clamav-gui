#include "scanoptionyn.h"
#define css "background-color:#404040;color:white"

scanoptionyn::scanoptionyn(QWidget *parent, QString setupFileName, QString section, bool checked, QString label, QString comment, QString language)
: QWidget(parent)
{
    m_ui.setupUi(this);
    m_setupFile = new setupFileHandler(setupFileName,this);
    m_setupFileSection = section;

    /*setupFileHandler * baseSetup = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini",this);
    QString languageset = baseSetup->getSectionValue("Setup","language");
    delete baseSetup;*/ // obsolete due to a static "getSectionValue" Method in setupFileHandler

    translator trans(language);

    m_option = label.mid(0,label.indexOf("<equal>"));
    //m_com = trans.translateit(comment);
    m_com = QCoreApplication::translate("ClamAV", comment.toUtf8().constData());
    m_ui.checkBox->setChecked(checked);
    m_ui.comboBox->setEnabled(checked);

    if (checked) {
        this->setStyleSheet(css);
    }

    m_ui.checkBox->setText(trans.beautifyString(m_com));
    m_ui.checkBox->setToolTip(m_option);

    if (label.indexOf("<equal>yes") != -1) m_ui.comboBox->setCurrentText("yes");else m_ui.comboBox->setCurrentText("no");
}

void scanoptionyn::slot_checkboxClicked(){
    if (m_ui.checkBox->isChecked() == false) {
        m_setupFile->removeKeyword(m_setupFileSection,m_option + "<equal>" + m_ui.comboBox->currentText());
        m_ui.comboBox->setEnabled(false);
        this->setStyleSheet("");
    } else {
        m_setupFile->setSectionValue(m_setupFileSection,m_option + "<equal>" + m_ui.comboBox->currentText(), m_com);
        m_ui.comboBox->setEnabled(true);
        this->setStyleSheet(css);
    }

    emit valuechanged();
}

void scanoptionyn::slot_comboboxChanged(QString value){
    if (m_ui.checkBox->isChecked() == true) {
        m_setupFile->removeKeyword(m_setupFileSection,m_option + "<equal>yes");
        m_setupFile->removeKeyword(m_setupFileSection,m_option + "<equal>no");
        m_setupFile->setSectionValue(m_setupFileSection,m_option + "<equal>" + value ,m_com);
    }

    emit valuechanged();
}
