#ifndef LECTER_H
#define LECTER_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QChar>
#include <QTextCursor>

class Letter
{
    // campi per gestire la lettera, la posizione, lo stile, etc
private:
    QChar letter;
    QVector<int> fractionalIndexes;
    QString letterID;
    QTextCharFormat format;
    QString username;
public:
    Letter(QChar letter, QVector<int> fractionals, QString letterID, QTextCharFormat format);
    Letter(const Letter& l);
    Letter& operator=(const Letter && source);
    Letter& operator=(const Letter& source);
    QChar getValue();
    QVector<int> getFractionalIndexes();
    QString getLetterID();
    int getSiteID();
    int getIndex();
    int getNumberOfFractionals();
    void setIndex(int index);
    void setStyle(QString style);
    void editIndex(int index, int value);
    void addFractionalDigit(int value);
    bool hasSameFractionals(Letter other);
    bool comesFirst(Letter other);
    void setFormat(QTextCharFormat format);
    void setStyleFromString(QString format);
    QString getUsername();
    QTextCharFormat getFormat();
    ~Letter(){}
};

#endif // LECTER_H
