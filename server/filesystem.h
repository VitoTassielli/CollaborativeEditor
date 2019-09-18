#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <map>
#include <string>
#include <QString>
#include <QTcpSocket>
#include <QDataStream>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "letter.h"
#include "filehandler.h"

class FileHandler;
class QTcpSocket;

class FileSystem
{
    static FileSystem* instance;
    std::map<QString, FileHandler*> files;
    FileSystem() {}
public:
    int sendFile(QString filename, QTcpSocket *socket);
    static FileSystem* getInstance();

};

#endif // FILESYSTEM_H