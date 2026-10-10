/*******************************************************************
 * Toolbox with procedures helping with the flatpak handling
*******************************************************************/
#include "toolbox.h"
#include "sharedvars.cpp"

bool isRunninginFlatPak()
{
    QString flatpakid = getenv("FLATPAK_ID");
    return !flatpakid.isEmpty()
    || QFile::exists("/.flatpak-info");
}

void startProcess(QProcess *process, QString basecommand, QStringList parameters)
{
    if (isRunninginFlatPak())
    {
        QStringList newParams;
        newParams << "--host" << basecommand;
        foreach (QString para, parameters)
            newParams << para;

        process->start("flatpak-spawn",newParams);
    }
    else {
        process->start(basecommand,parameters);
    }
}

bool checkFileExists(const QString path)
{
    bool rc = false;

    if (isRunninginFlatPak())
    {
        QProcess process;

        process.start("flatpak-spawn", {"--host","test","-f",path});

        if (!process.waitForFinished(3000))
            return false;

        rc =  process.exitStatus() == QProcess::NormalExit
               && process.exitCode() == 0;
    }
    else
        rc = QFileInfo::exists(path);

    return rc;
}

bool processRunning(QString progname)
{
    QProcess process;

    if (isRunninginFlatPak())
        process.start("flatpak-spawn", {"--host","ps","-f",progname});
    else
        process.start("ps", {"-f",progname});

    if (!process.waitForFinished(3000))
        return false;

    return process.exitStatus() == QProcess::NormalExit
         && process.exitCode() == 0;
}

QString pidof(QString progname)
{
    QProcess process;

    if (isRunninginFlatPak())
    {
        QString command = "pidof -s " + progname;
        process.start("flatpak-spawn", {"--host","bash","-c",command});

    }
    else {
        QString command = "pidof -s " + progname;
        process.start("bash", {"-c",command});
    }

    if (!process.waitForFinished(3000))
        return "";

    return QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
}

QString which(QString progname)
{
    QProcess process;

    if (isRunninginFlatPak())
        process.start("flatpak-spawn", {"--host","which",progname});
    else
        process.start("which", {progname});

    if (!process.waitForFinished(3000))
        return "";

    return QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
}

QString whoami()
{
    QProcess process;

    if (isRunninginFlatPak())
        process.start("flatpak-spawn", {"--host","whoami"});
    else
        process.start("whoami", QStringList());

    if (!process.waitForFinished(3000))
        return "";

    return QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
}

QString runProg(QString command, QStringList parameters)
{
    QProcess process;
    if (isRunninginFlatPak())
    {
        QStringList newParams;
        newParams << "--host" << command;
        foreach (QString para, parameters)
            newParams << para;

        process.start("flatpak-spawn",newParams);
    }
    else
        process.start(command,parameters);

    if (!process.waitForFinished(3000))
        return "";

    return QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
}

void assembleScanParameters(setupFileHandler * m_setupFile, QStringList * parameters) {
    QStringList selectedOptions = m_setupFile->getKeywords("SelectedOptions");
    QStringList scanLimitations = m_setupFile->getKeywords("ScanLimitations");
    QStringList directoryOptions = m_setupFile->getKeywords("Directories");
    QString option;
    QString checked;
    QString value;

    for (int i = 0; i < selectedOptions.count(); i++)
    {
        *parameters << selectedOptions.at(i).left(selectedOptions.indexOf("|")).replace("<equal>", "=");
    }

    // Directory Options
    for (int i = 0; i < directoryOptions.count(); i++)
    {
        option = directoryOptions.at(i);
        value = m_setupFile->getSectionValue("Directories", option);
        checked = value.left(value.indexOf("|"));
        value = value.mid(value.indexOf("|") + 1);

        if ((checked == "checked") && (value != ""))
        {
            for (int idx = 0; idx < directoryOptionKeywords.size(); idx++)
            {
                if (directoryOptionKeywords.at(idx) == option)
                {
                    if (directoryOptionKeywords.at(idx) == "ScanReportToFile")
                    {
                        if (value != "")
                        {
                            *parameters << "--log=" + value;
                            QFile file(value);
                            if (file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
                            {
                                QTextStream stream(&file);
                                stream << "\n<Scanning startet> " << QDateTime::currentDateTime().toString("yyyy/M/d - hh:mm");
                                file.close();
                            }
                        }
                    }
                    else
                        *parameters << directoryOptionSwitches.at(idx) + "=" + value;
                }
            }
        }
    }

    // Scan Limitations
    for (int i = 0; i < scanLimitations.count(); i++)
    {
        option = scanLimitations.at(i);
        value = m_setupFile->getSectionValue("ScanLimitations", option);
        checked = value.left(value.indexOf("|"));
        value = value.mid(value.indexOf("|") + 1);
        if (checked == "checked")
        {
            for (int i = 0; i < scanLimitKeywords.length(); i++)
            {
                if (option == scanLimitKeywords.at(i))
                    *parameters << scanLimitSwitches.at(i) + "=" + value;
            }
        }
    }

    // REGEXP and Include Exclude Options
    for (int idx = 0; idx < inclExclKeywords.size(); idx++)
    {
        if (idx < 4)
        {
            value = m_setupFile->getSectionValue("REGEXP_and_IncludeExclude",inclExclKeywords.at(idx));
            checked = value.left(value.indexOf("|"));
            value = value.mid(value.indexOf("|") + 1);
            if (checked == "checked") *parameters << inclExclSwitches.at(idx) + "=" + value;
        }
        else {
            if (m_setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","EnablePUAOptions") == true)
            {
                if ((m_setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude",inclExclKeywords.at(idx)) == true) &&
                    (inclExclKeywords.at(idx) != "EnablePUAOptions"))
                    *parameters << inclExclSwitches.at(idx);
            }
        }
    }

}

bool isRunninginAppImage()
{
    QString AppImagePath = qEnvironmentVariable("APPIMAGE");
    bool rc = false;

    if (AppImagePath != "")
        rc = true;

    return rc;
}

bool createServiceMenus()
{
    bool created = false;

    if (which("nemo") != "")
    {
        addServiceMenuNemo();
        created = true;
    }
    if (which("dolphin") != "")
    {
        addServiceMenuDolphin();
        created = true;
    }
    if (which("gnome-commander") != "")
    {
        addServiceMenuGnomeCommander();
        created = true;
    }

    return created;
    //*****************************************************************************
}

bool addServiceMenuNemo(){
    bool created = false;
    QDir mkpathDir(QDir::homePath());

    if (QFileInfo::exists(QDir::homePath() + "/.local/share/nemo/actions") == false)
        mkpathDir.mkpath(QDir::homePath() + "/.local/share/nemo/actions");

    // Service Menu for NEMO
    if (QFileInfo::exists(QDir::homePath() + "/.local/share/nemo/actions"))
    {
        setupFileHandler* serviceFile = new setupFileHandler(QDir::homePath() + "/.local/share/nemo/actions/scan.nemo_action", nullptr);
        serviceFile->setSectionValue("Nemo Action", "Name", "scan with ClamAV-GUI");
        serviceFile->setSectionValue("Nemo Action", "Comment", "scan with ClamAV-GUI");
        if (isRunninginFlatPak())
            serviceFile->setSectionValue("Nemo Action", "Exec", "flatpak run --branch=master --arch=x86_64 --command=clamav-gui io.github.wusel1007.clamav-gui --scan %F");
        else
            serviceFile->setSectionValue("Nemo Action", "Exec", "clamav-gui --scan %F");
        //serviceFile->setSectionValue("Nemo Action", "Exec", "clamav-gui --scan %F");
        serviceFile->setSectionValue("Nemo Action", "Icon-Name", "clamav-gui");
        serviceFile->setSectionValue("Nemo Action", "Selection", "notnone");
        serviceFile->setSectionValue("Nemo Action", "Extensions", "any");
        serviceFile->setSectionValue("Nemo Action", "Quote", "double");
        delete serviceFile;
        created = true;
    }
    return created;
}

bool removeServiceMenuNemo() {
    if (checkFileExists(QDir::homePath() + "/.local/share/nemo/actions/scan.nemo_action"))
    {
        QFile file(QDir::homePath() + "/.local/share/nemo/actions/scan.nemo_action");
        return file.remove();
    }
    return true;
}

bool addServiceMenuGnomeCommander() {
    bool created = false;
    // ServiceMenu for GNOME-Commander
    QStringList gnomecommanderParams;
    gnomecommanderParams << "get"  << "org.gnome.gnome-commander.preferences.general" << "favorite-apps";
    QString output = runProg("gsettings",gnomecommanderParams);
    gnomecommanderParams.clear();

    if (output.indexOf("[]") != -1)
    {
        if (isRunninginFlatPak())
            gnomecommanderParams << "set"  << "org.gnome.gnome-commander.preferences.general" << "favorite-apps" << "[('scan with ClamAV-GUI', 'flatpak run --branch=master --arch=x86_64 --command=clamav-gui io.github.wusel1007.clamav-gui --scan %F', '/usr/share/icons/hicolor/48x48/apps/clamav-gui.png', '', uint32 2, false, true, false)]";
        else
            gnomecommanderParams << "set"  << "org.gnome.gnome-commander.preferences.general" << "favorite-apps" << "[('scan with ClamAV-GUI', '/usr/bin/clamav-gui --scan %F', '/usr/share/icons/hicolor/48x48/apps/clamav-gui.png', '', uint32 2, false, true, false)]";

        QProcess::execute("gsettings",gnomecommanderParams);
    }
    else {
        if (output.indexOf("clamav-gui") == -1)
        {
            if (isRunninginFlatPak())
                output = output.mid(0,output.length() - 1) + ", ('scan with ClamAV-GUI', 'flatpak run --branch=master --arch=x86_64 --command=clamav-gui io.github.wusel1007.clamav-gui --scan %F', '/usr/share/icons/hicolor/48x48/apps/clamav-gui.png', '', uint32 2, false, true, false)]";
            else
                output = output.mid(0,output.length() - 1) + ", ('scan with ClamAV-GUI', '/usr/bin/clamav-gui --scan %F', '/usr/share/icons/hicolor/48x48/apps/clamav-gui.png', '', uint32 2, false, true, false)]";
        }
        gnomecommanderParams << "set" << "org.gnome.gnome-commander.preferences.general" << "favorite-apps"  << output;
        QProcess::execute("gsettings",gnomecommanderParams);
    }
    created = true;
    //*****************************************************************************
    return created;
}

bool removeServiceMenuGnomeCommander()
{
    bool removed = false;
    // ServiceMenu for GNOME-Commander
    QStringList gnomecommanderParams;
    gnomecommanderParams << "get"
                         << "org.gnome.gnome-commander.preferences.general"
                         << "favorite-apps";

    QString output = runProg("gsettings", gnomecommanderParams);

    if (output != "")
    {
        int start = output.indexOf("('scan with ClamAV-GUI'");
        if (start != -1)
        {
            int end = output.indexOf(")",start);
            QString basereplacer = output.mid(start,end-start + 1);

            QString replacer = ", " + basereplacer + ", ";
            if (output.indexOf(replacer) == -1)
                replacer = basereplacer + ", ";
            if (output.indexOf(replacer) == -1)
                replacer = basereplacer;

            if (output.indexOf(replacer) != -1)
            {
                output = output.replace(replacer,"");
                gnomecommanderParams.clear();
                gnomecommanderParams << "set"
                                     << "org.gnome.gnome-commander.preferences.general"
                                     << "favorite-apps"
                                     << output;

                if (QProcess::execute("gsettings", gnomecommanderParams) == 0)
                    removed = true;
            }
            else
            {
                // Der Eintrag existiert bereits nicht mehr.
                removed = true;
            }
        }
    }

    return removed;
}

bool addServiceMenuDolphin(){
    bool created = false;
    QDir mkpathDir(QDir::homePath());

    //*****************************************************************************
    //creating service Menu for Dolphin
    //*****************************************************************************
    QString serviceMenuPath;
    if (QFileInfo::exists(QDir::homePath() + "/.local/share/kservices5/ServiceMenus"))
        serviceMenuPath = QDir::homePath() + "/.local/share/kservices5/ServiceMenus";

    if (serviceMenuPath.isEmpty())
        serviceMenuPath = QDir::homePath() + "/.local/share/kio/servicemenus";

    if (!QFileInfo::exists(serviceMenuPath))
        mkpathDir.mkpath(serviceMenuPath);

    if (QFileInfo::exists(serviceMenuPath) == true)
    {
        setupFileHandler* serviceFile = new setupFileHandler(serviceMenuPath + "/scanWithClamAV-GUI.desktop", nullptr);
        serviceFile->setSectionValue("Desktop Entry", "Type", "Service");
        serviceFile->setSectionValue("Desktop Entry", "ServiceTypes", "KonqPopupMenu/Plugin");
        serviceFile->setSectionValue("Desktop Entry", "MimeType", "all/all;");
        serviceFile->setSectionValue("Desktop Entry", "Actions", "scan;");
        serviceFile->setSectionValue("Desktop Entry", "Icon", "clamav-gui");
        serviceFile->setSectionValue("Desktop Entry", "X-KDE-Priority", "TopLevel");
        serviceFile->setSectionValue("Desktop Entry", "X-KDE-StartupNotify", "false");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu", "Scan with ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[de]", "Scannen mit ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[da_DK]", "Scannen med ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[es_ES]", "Analizar con ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[us]", "Scan with ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[gb]", "Scan with ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[pt]", "Investigar com ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[br]", "Investigar com ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[pt_BR]", "Investigar com ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[fr]", "Scanner avec ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[it]", "Scansione con ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry", "NO-X-KDE-Submenu[uk]", "Сканування за допомогою ClamAV-GUI");

        serviceFile->setSectionValue("Desktop Action scan", "Name", "scan");
        serviceFile->setSectionValue("Desktop Action scan", "Name[de]", "Scannen mit ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[es_ES]", "Analizar con ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[us]", "Scan with ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[gb]", "Scan with ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[pt]", "Investigar com ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[br]", "Investigar com ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[fr]", "Scanner avec ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[it]", "Scansione con ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Name[uk]", "Сканування за допомогою ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Action scan", "Icon", "clamav-gui");
        if (isRunninginFlatPak())
            serviceFile->setSectionValue("Desktop Action scan", "Exec", "flatpak run --branch=master --arch=x86_64 --command=clamav-gui io.github.wusel1007.clamav-gui --scan %F");
        else
            serviceFile->setSectionValue("Desktop Action scan", "Exec", "clamav-gui --scan %F");
        delete serviceFile;

        QFile file(serviceMenuPath + "/scanWithClamAV-GUI.desktop");
        file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadGroup |
                            QFileDevice::WriteGroup | QFileDevice::ExeGroup);
        created = true;
    }

    return created;
}

bool removeServiceMenuDolphin() {
    bool removed = false;
    QString serviceMenuPath;
    if (QFileInfo::exists(QDir::homePath() + "/.local/share/kservices5/ServiceMenus"))
        serviceMenuPath = QDir::homePath() + "/.local/share/kservices5/ServiceMenus";

    if (serviceMenuPath.isEmpty() && QFileInfo::exists(QDir::homePath() + "/.local/share/kio/servicemenus"))
        serviceMenuPath = QDir::homePath() + "/.local/share/kio/servicemenus";

    if (serviceMenuPath != "")
    {
        if (QFileInfo::exists(serviceMenuPath + "/scanWithClamAV-GUI.desktop") == true)
        {
            QFile file(serviceMenuPath + "/scanWithClamAV-GUI.desktop");
            file.remove();
            removed = !QFileInfo::exists(serviceMenuPath + "/scanWithClamAV-GUI.desktop");
        }
    }
    return removed;
}

bool serviceMenuConfigPresent(QString filemanager)
{
    bool rc = false;

    if (filemanager == "dolphin")
    {
        if ((checkFileExists(QDir::homePath() + "/.local/share/kservices5/ServiceMenus/scanWithClamAV-GUI.desktop")) || (checkFileExists(QDir::homePath() + "/.local/share/kio/servicemenus/scanWithClamAV-GUI.desktop")))
            rc = true;
    }

    if (filemanager == "nemo")
    {
        if (checkFileExists(QDir::homePath() + "/.local/share/nemo/actions/scan.nemo_action"))
            rc = true;
    }

    if (filemanager == "gnome-commander")
    {
        QStringList gnomecommanderParams;
        gnomecommanderParams << "get"
                             << "org.gnome.gnome-commander.preferences.general"
                             << "favorite-apps";

        QString output = runProg("gsettings", gnomecommanderParams);

        if (output != "")
        {
            if (output.indexOf("scan with ClamAV-GUI") != -1)
                rc = true;
        }
    }

    return rc;
}

QString beautifyString(QString value, int length)
{
    QString helper = value;
    QString rc = "";
    int counter = 0;

    // Word-Wrap of lines that are longer than [length] characters ...
    for (int i = 0; i < helper.length(); i++)
    {
        if ((counter > length) && (helper.mid(i,1) == ' '))
        {
            rc = rc + "\n";
            counter = 0;
        }
        else {
            rc = rc + helper.mid(i,1);
        }
        counter++;
    }

    return rc;
}

QString getClamAVVersion()
{
    QString buffer = runProg("freshclam", {"--config-file", QString(QDir::homePath() + "/.clamav-gui/freshclam.conf"), "-V"});
    QStringList versionSections = buffer.split("/");
    while (versionSections.length() < 3)
        versionSections << "n/a";
    QString scannerVersion = versionSections.at(0).mid(6);
    QString value = versionSections.at(1);
    QString lastUpdate = versionSections.at(2);

    QString systemInfo = "<div style='font-size:12px;line-height:20px;'><b>Scanner: <font color='navy'>" + scannerVersion +
                         "</font><br>Database: <font color='navy'>" + value + "</font><br>";
    systemInfo += "Date: <font color='navy'>" + lastUpdate + "</font></b></div>";
    return systemInfo;
}
