#ifndef TOOLBOX_H
#define TOOLBOX_H
#include <QtEnvironmentVariables>
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
void startProcess(QProcess * process,QString basecommand,QStringList parameters);
QString runProg(QString basecommand,QStringList parameters);
bool checkFileExists(const QString path);
bool processRunning(QString);
QString pidof(QString progname);
QString which(QString progname);
QString whoami();
#endif  // TOOLBOX_H
