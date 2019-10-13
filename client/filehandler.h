#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QObject>
#include <QVector>
#include <QJsonArray>
#include "letter.h"

class FileHandler
{
private:
    QVector<QString> listFiles;
    QVector<Letter> letters;
    int siteCounter;

    QVector<int> calculateInternalIndex(QVector<int> prevPos, QVector<int> nextPos);
    //void insertLetterInArray(Letter *newLetter);

public:
    FileHandler(int siteid);
    void localInsert(int externalIndex, QChar newLetterValue, int clientID);
    void localDelete(int externalIndex);
    void remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter);
    void remoteDelete(QString deletedLetterID);
    void setListFiles(QVector<QString> listFiles);
    void insertLetters(QVector<Letter> letters);
};

#endif // FILEHANDLER_H