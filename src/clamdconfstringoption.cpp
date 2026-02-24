#include "clamdconfstringoption.h"
#define css_mono "background-color:#404040;color:white"

clamdConfStringOption::clamdConfStringOption(QWidget* parent, QString keyword, bool checked, QString label, QString options, QString language,
                                             setupFileHandler* setupFile)
    : QWidget(parent), m_optionKeyword(keyword), m_optionValue(options), m_setupFile(setupFile)
{
    m_startup = true;
    translator trans(language);

    m_ui.setupUi(this);

    if (m_setupFile->singleLineExists(keyword) == true) {
        m_ui.checkBox->setChecked(true);
        m_ui.lineEdit->setText(m_setupFile->getSingleLineValue(keyword));
    }
    else {
        m_ui.checkBox->setChecked(checked);
        m_ui.lineEdit->setText(options);
    }

    //label = trans.translateit(label);
    label = QCoreApplication::translate("ClamAV", label.toUtf8().constData());
    label = translator::beautifyString(label, 120);

    m_ui.checkBox->setText(label);

    m_startup = false;
    slot_checkBoxClicked();
}

void clamdConfStringOption::slot_checkBoxClicked()
{
    if (m_startup == false) {
        bool state = m_ui.checkBox->isChecked();
        m_ui.lineEdit->setEnabled(state);
        if (state == true) {
            QString value = m_setupFile->getSingleLineValue(m_optionKeyword);
            m_setupFile->removeSingleLine(m_optionKeyword, value);
            m_setupFile->setSingleLineValue(m_optionKeyword, m_ui.lineEdit->text());
        }
        else {
            QString value = m_setupFile->getSingleLineValue(m_optionKeyword);
            m_setupFile->removeSingleLine(m_optionKeyword, value);
        }
        emit settingChanged();
        (state == true) ? m_ui.frame->setStyleSheet(css_mono) : m_ui.frame->setStyleSheet("");
    }
}

void clamdConfStringOption::slot_lineEditChanged()
{
    if (m_startup == false) {
        if (m_ui.checkBox->isChecked() == true) {
            QString value = m_setupFile->getSingleLineValue(m_optionKeyword);
            m_setupFile->removeSingleLine(m_optionKeyword, value);
            m_setupFile->setSingleLineValue(m_optionKeyword, m_ui.lineEdit->text());
            emit settingChanged();
        }
    }
}
