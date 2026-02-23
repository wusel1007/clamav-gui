#include "translator.h"
#include <QCoreApplication>
#include <QTranslator>
#include <QDebug>

translator::translator(QString lang)
{
    lang = lang.replace("[", "").replace("]", "");
    m_languageset = lang;
    loadTranslations();
}

void translator::loadTranslations()
{
    // Map language codes to QM file names
    QMap<QString, QString> languageMap;
    languageMap["de_DE"] = "clamav-de_DE.qm";
    languageMap["pt_PT"] = "clamav-pt_PT.qm";
    languageMap["it_IT"] = "clamav-it_IT.qm";
    languageMap["da_DK"] = "clamav-da_DK.qm";
    languageMap["fr_FR"] = "clamav-fr_FR.qm";
    languageMap["es_ES"] = "clamav-es_ES.qm";
    languageMap["uk_UA"] = "clamav-uk_UA.qm";
    languageMap["zh_CN"] = "clamav-zh_CN.qm";
    
    QString qmFile = languageMap.value(m_languageset, "");
    
    if (!qmFile.isEmpty()) {
        // Try to load from various locations
        QStringList searchPaths;
        searchPaths << qmFile;
        searchPaths << "translations/" + qmFile;
        searchPaths << ":/translations/" + qmFile;
        searchPaths << QCoreApplication::applicationDirPath() + "/../share/clamav-gui/" + qmFile;
        
        bool loaded = false;
        for (const QString& path : searchPaths) {
            qDebug() << "path" << path; 
            if (m_translator.load(path)) {
                QCoreApplication::installTranslator(&m_translator);
                qDebug() << "Loaded translations from:" << path;
                loaded = true;
                break;
            }
        }
        
        if (!loaded) {
            qDebug() << "Could not load translations for language:" << m_languageset;
        }
    }
}

QString translator::beautifyString(QString value, int length) {
    QString helper = value;
    QString rc = "";
    int counter = 0;

    // Word-Wrap of lines that are longer than [length] characters ...
    for (int i = 0; i < helper.length(); i++) {
        if ((counter > length) && (helper.mid(i,1) == ' ')) {
            rc = rc + "\n";
            counter = 0;
        } else {
            rc = rc + helper.mid(i,1);
        }
        counter++;
    }

    return rc;
}

QString translator::translateit(QString original) {
    // Use Qt's translation system - all translations come from TS files
    // QCoreApplication::translate() is available without QObject inheritance
    return QCoreApplication::translate("ClamAV", original.toUtf8().constData());
}