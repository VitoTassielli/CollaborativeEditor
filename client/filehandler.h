#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QObject>
#include <QVector>
#include <QJsonArray>
#include "letter.h"

class FileHandler : public QObject
{
    Q_OBJECT

private:
    int fileid;
    QVector<Letter*> letters;
    int siteCounter;
    int size;
    QString URI;
    QVector<int> calculateInternalIndex(QVector<int> prevPos, QVector<int> nextPos);

public:
    explicit FileHandler(QObject *parent = nullptr);
    void setValues(QVector<Letter*> letters);
    QVector<Letter*> getVectorFile();
    int getFileId();
    void setFileId(int fileid);
    int getSize();
    void setSize(int size);
    int getSiteCounter();
    void setSiteCounter(int siteCounter);
    void setURI(QString URI);
    QString getURI();

public slots:
    void localInsert(int externalIndex, QChar newLetterValue, int clientID, bool isBold, bool isUnderlined, bool isItalic);
    void localDelete(int externalIndex);
    void remoteInsert(QJsonArray position, QChar newLetterValue, int externalIndex, int siteID, int siteCounter);
    void remoteDelete(QString deletedLetterID);
    void changeStyle(QString type, bool newValue, int startPos, int endPos);

signals:
    void localInsertNotify(QChar newLetterValue, QJsonArray position, int siteID, int siteCounter, int externalIndex);
    void localDeleteNotify(QString deletedLetterID, int fileid, int siteCounter);
    void readyRemoteInsert(QChar newLetter, int externalIndex);
    void readyRemoteDelete(int externalIndex);
};

#endif // FILEHANDLER_H
