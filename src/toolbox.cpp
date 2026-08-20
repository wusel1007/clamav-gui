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
