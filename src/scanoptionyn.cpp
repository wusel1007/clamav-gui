/************************************************************************
 * Checkable Scan Option element for the scan option tab with yes/no
 * combobox.
 ************************************************************************/
#include "scanoptionyn.h"
#define css "background-color:#404040;color:white"

scanoptionyn::scanoptionyn(QWidget *parent, QString setupFileName, QString section, bool checked, QString label, QString comment)
: scanOptionBaseClass(parent)
{
    m_ui.setupUi(this);
    m_setupFile = new setupFileHandler(setupFileName,this);
    m_setupFileSection = section;

    m_option = label.mid(0,label.indexOf("<equal>"));
// -------------------------------------------------------------------------
// For DEBUG reasons only.
/*    QFile file(QDir::homePath() + "/clamav-xx_XX.ts");
    if (file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
    {
        QTextStream stream(&file);
        stream << "    <message>\n        <source>" << comment << "</source>\n        <translation>TRANS</translation>\n    </message>\n";
        file.close();
    }*/
// -------------------------------------------------------------------------

    m_com = QCoreApplication::translate("ClamAV", comment.toUtf8().constData());
    m_ui.checkBox->setChecked(checked);
    m_ui.comboBox->setEnabled(checked);

    if (checked)
        this->setStyleSheet(css);

    m_ui.checkBox->setText(beautifyString(m_com));
    m_ui.checkBox->setToolTip(m_option);

    if (label.indexOf("<equal>yes") != -1) m_ui.comboBox->setCurrentText("yes");else m_ui.comboBox->setCurrentText("no");
}

QString scanoptionyn::getOption()
{
    return m_option;
}

QString scanoptionyn::getComment()
{
    return m_com;
}

bool scanoptionyn::isChecked()
{
    return m_ui.checkBox->isChecked();
}

void scanoptionyn::setCurrentText(QString text)
{
    m_ui.comboBox->setCurrentText(text);
}

void scanoptionyn::slot_checkboxClicked(){
    if (m_ui.checkBox->isChecked() == false)
    {
        m_setupFile->removeKeyword(m_setupFileSection,m_option + "<equal>" + m_ui.comboBox->currentText());
        m_ui.comboBox->setEnabled(false);
        this->setStyleSheet("");
    }
    else {
        m_setupFile->setSectionValue(m_setupFileSection,m_option + "<equal>" + m_ui.comboBox->currentText(), m_com);
        m_ui.comboBox->setEnabled(true);
        this->setStyleSheet(css);
    }

    emit valuechanged();
}

void scanoptionyn::slot_comboboxChanged(QString value){
    if (m_ui.checkBox->isChecked() == true)
    {
        m_setupFile->removeKeyword(m_setupFileSection,m_option + "<equal>yes");
        m_setupFile->removeKeyword(m_setupFileSection,m_option + "<equal>no");
        m_setupFile->setSectionValue(m_setupFileSection,m_option + "<equal>" + value ,m_com);
    }

    emit valuechanged();
}
