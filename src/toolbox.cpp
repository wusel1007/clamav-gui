#include "toolbox.h"

bool isRunninginFlatPak()
{
    return !qEnvironmentVariable("FLATPAK_ID").isEmpty()
    || QFile::exists("/.flatpak-info");
}

void startProcess(QProcess *process, QString basecommand, QStringList parameters)
{
    if (isRunninginFlatPak())
    {
        QStringList newParams;
        newParams << "--host" << basecommand;
        foreach (QString para, parameters) {
            newParams << para;
        }
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

        if (!process.waitForFinished(3000)) {
            return false;
        }

        rc =  process.exitStatus() == QProcess::NormalExit
               && process.exitCode() == 0;
    }
    else {
        rc = QFileInfo::exists(path);
    }

    return rc;
}

bool processRunning(QString progname)
{
    QProcess process;

    if (isRunninginFlatPak())
    {
        process.start("flatpak-spawn", {"--host","ps","-f",progname});
    }
    else {
        process.start("ps", {"-f",progname});
    }

    if (!process.waitForFinished(3000)) {
        return false;
    }

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

    if (!process.waitForFinished(3000)) {
        return "";
    }

    return QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
}

QString which(QString progname)
{
    QProcess process;

    if (isRunninginFlatPak())
    {
        process.start("flatpak-spawn", {"--host","which",progname});

    }
    else {
        process.start("which", {progname});
    }

    if (!process.waitForFinished(3000)) {
        return "";
    }

    return QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
}

QString whoami()
{
    QProcess process;

    if (isRunninginFlatPak())
    {
        process.start("flatpak-spawn", {"--host","whoami"});

    }
    else {
        process.start("whoami", {});
    }

    if (!process.waitForFinished(3000)) {
        return "";
    }

    return QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
}

QString runProg(QString command, QStringList parameters)
{
    QProcess process;
    if (isRunninginFlatPak())
    {
        QStringList newParams;
        newParams << "--host" << command;
        foreach (QString para, parameters) {
            newParams << para;
        }
        process.start("flatpak-spawn",newParams);
    }
    else {
        process.start(command,parameters);
    }

    if (!process.waitForFinished(3000)) {
        return "";
    }

    return QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
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
    //*****************************************************************************
    //creating service Menu for Dolphin
    //*****************************************************************************
    QString serviceMenuPath;
    if (QFileInfo::exists(QDir::homePath() + "/.local/share/kservices5/ServiceMenus"))
        serviceMenuPath = QDir::homePath() + "/.local/share/kservices5/ServiceMenus";

    if (serviceMenuPath.isEmpty() && QFileInfo::exists(QDir::homePath() + "/.local/share/kio/servicemenus"))
        serviceMenuPath = QDir::homePath() + "/.local/share/kio/servicemenus";

    /*if ((serviceMenuPath.isEmpty()) && (QFileInfo::exists(QCoreApplication::applicationDirPath() + "/../share/" + "kio/servicemenues")))
        serviceMenuPath = QDir::homePath() + "/.local/share/kio/servicemenus";

    if ((serviceMenuPath.isEmpty()) && (QFileInfo::exists(QCoreApplication::applicationDirPath() + "/../share/" + "kservices5/ServiceMenus")))
        serviceMenuPath = QDir::homePath() + "/.local/share/kservices5/ServiceMenus";*/

    if (serviceMenuPath != "")
    {
        if (!QFileInfo::exists(serviceMenuPath))
        {
            QDir dir(serviceMenuPath);
            dir.mkpath(serviceMenuPath);
        }
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
    // Service Menu for NEMO
    if (QFileInfo::exists(QDir::homePath() + "/.local/share/nemo/actions"))
    {
        setupFileHandler* serviceFile = new setupFileHandler(QDir::homePath() + "/.local/share/nemo/actions/scan.nemo_action", nullptr);
        serviceFile->setSectionValue("Nemo Action", "Name", "scan with ClamAV-GUI");
        serviceFile->setSectionValue("Nemo Action", "Comment", "scan with ClamAV-GUI");
        serviceFile->setSectionValue("Nemo Action", "Exec", "clamav-gui --scan &F");
        serviceFile->setSectionValue("Nemo Action", "Icon-Name", "clamav-gui");
        serviceFile->setSectionValue("Nemo Action", "Selection", "notnone");
        serviceFile->setSectionValue("Nemo Action", "Extensions", "any");
        serviceFile->setSectionValue("Nemo Action", "Separator", ",");
        serviceFile->setSectionValue("Nemo Action", "Dependencies", "clamav-gui");
        delete serviceFile;
    }

    return created;
    //*****************************************************************************
}
