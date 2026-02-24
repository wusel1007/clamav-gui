#include "clamdconfspinboxoption.h"
#define css_mono "background-color:#404040;color:white"

clamdconfspinboxoption::clamdconfspinboxoption(QWidget* parent, QString keyword, bool checked, QString label, QString options, QString language,
                                               setupFileHandler* setupFile)
    : QWidget(parent), m_optionKeyword(keyword), m_setupFile(setupFile)
{
    translator trans(language);

    m_ui.setupUi(this);
    m_ui.checkBox->setChecked(checked);

    QStringList values = options.split(",");
    int spinvalue;
    int spinmin;
    int spinmax;
    spinmin = values.at(0).toInt();  //helper.toInt();
    spinmax = values.at(1).toInt();
    spinvalue = values.at(2).toInt();

    if (spinvalue > 1048576) {
        spinmin = spinmin / 1048576;
        spinmax = spinmax / 1048576;
        spinvalue = spinvalue / 1048576;
    }
    m_ui.spinBox->setMinimum(spinmin);
    m_ui.spinBox->setMaximum(spinmax);
    connect(m_ui.checkBox, SIGNAL(clicked(bool)), this, SLOT(slot_checkboxClicked()));

    if (m_setupFile->singleLineExists(m_optionKeyword) == true) {
        m_ui.checkBox->setChecked(true);
        QString valueHelper = m_setupFile->getSingleLineValue(m_optionKeyword);
        if (valueHelper.indexOf("M") != -1) {
            m_ui.label->setText("MB");
            valueHelper = valueHelper.replace("M", "");
        }
        m_ui.spinBox->setValue(valueHelper.toInt());
    }
    else {
        m_ui.spinBox->setValue(spinvalue);
    }

    //label = trans.translateit(label);
    label = QCoreApplication::translate("ClamAV", label.toUtf8().constData());
    m_ui.checkBox->setText(translator::beautifyString(label, 120));
    m_startup = false;

    slot_checkboxClicked();
}

void clamdconfspinboxoption::slot_checkboxClicked()
{
    if (m_startup == false) {
        bool state = m_ui.checkBox->isChecked();
        m_ui.spinBox->setEnabled(state);
        m_ui.label->setEnabled(state);
        (state == true) ? m_ui.frame->setStyleSheet(css_mono) : m_ui.frame->setStyleSheet("");
        if (state == true) {
            if (m_ui.label->text() == "MB") {
                m_setupFile->setSingleLineValue(m_optionKeyword, QString::number(m_ui.spinBox->value()) + "M");
            }
            else {
                m_setupFile->setSingleLineValue(m_optionKeyword, QString::number(m_ui.spinBox->value()));
            }
        }
        else {
            QString value = m_setupFile->getSingleLineValue(m_optionKeyword);
            m_setupFile->removeSingleLine(m_optionKeyword, value);
        }
        emit settingChanged();
    }
}

void clamdconfspinboxoption::slot_spinboxChanged()
{
    if (m_startup == false) {
        bool state = m_ui.checkBox->isChecked();
        if (state == true) {
            QString value = m_setupFile->getSingleLineValue(m_optionKeyword);
            m_setupFile->removeSingleLine(m_optionKeyword, value);
            if (m_ui.label->text() == "MB") {
                m_setupFile->setSingleLineValue(m_optionKeyword, QString::number(m_ui.spinBox->value()) + "M");
            }
            else {
                m_setupFile->setSingleLineValue(m_optionKeyword, QString::number(m_ui.spinBox->value()));
            }
            emit settingChanged();
        }
    }
}
