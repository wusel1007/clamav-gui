#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QMessageBox>
#include <QStringList>
#include <QString>
#include <QTranslator>

/*!
 * \brief The translator class provides translation services for the application.
 * It can translate strings based on the selected language and also provides a method to beautify strings.
 * TODO: We should consider to remove this class in the future, as Qt provides a better way to handle translations.
 */
class translator
{
public:
    /*!
     * \brief translator Constructor that initializes the translator with a language set.
     * \param lang The language to be used for translation.
     */
    explicit translator(QString lang);

    /*!
        * \brief translateit Translates the given string based on the selected language.
        * \param original The original string to be translated.
        * \return The translated string.
    */
    QString translateit(QString original);

    /*!
     * \brief beautifyString Beautifies the given string by truncating it to the specified length and adding ellipsis if needed.
     * \param value The original string to be beautified.
     * \param length The maximum length of the beautified string.
     * \return The beautified string.
     */
    static QString beautifyString(QString value, int length = 50);

private:
    void loadTranslations();  // Add this declaration
    QTranslator m_translator;
    QString m_languageset;
};

#endif // TRANSLATOR_H
