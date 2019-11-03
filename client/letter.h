#ifndef LECTER_H
#define LECTER_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QChar>

class Letter
{
    // campi per gestire la lettera, la posizione, lo stile, etc
private:
    QChar letter;
    QVector<int> fractionalIndexes;
    QString letterID;

public:
    Letter(QChar letter, QVector<int> fractionals, QString letterID);
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
    void editIndex(int index, int value);
    void addFractionalDigit(int value);
    bool hasSameFractionals(Letter other);
    bool comesFirst(Letter other);
    ~Letter(){}
};

#endif // LECTER_H
