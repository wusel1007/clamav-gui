#ifndef TOOLBOX_H
#define TOOLBOX_H
//#include <QtEnvironmentVariables>
#include <cstdlib>
#include <QDateTime>
#include <QStringList>
#include <QFileInfo>
#include <QProcess>
#include <QString>
#include <QFile>
#include <QDir>

#include "setupfilehandler.h"

bool isRunninginFlatPak();
bool isRunninginAppImage();
bool createServiceMenus();
bool addServiceMenuGnomeCommander();
bool addServiceMenuDolphin();
bool addServiceMenuNemo();
bool removeServiceMenuGnomeCommander();
bool removeServiceMenuDolphin();
bool removeServiceMenuNemo();
bool serviceMenuConfigPresent(QString);
void startProcess(QProcess * process,QString basecommand,QStringList parameters);
QString runProg(QString basecommand,QStringList parameters);
void assembleScanParameters(setupFileHandler * m_setupFile, QStringList * parameters);
bool checkFileExists(const QString path);
bool processRunning(QString);
QString pidof(QString progname);
QString which(QString progname);
QString whoami();
QString getClamAVVersion();
QString beautifyString(QString value, int length = 50);
#endif  // TOOLBOX_H
