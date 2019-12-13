#include "letter.h"

Letter::Letter(QChar letter, QVector<int> fractionals, QString letterID) : letter(letter), fractionalIndexes(fractionals), letterID(letterID) {
    //this->fractionalIndexes.insert(0, index);
    // index già presente nella posizione 0 di fractionals
}

//Aggiornamento stili
Letter::Letter(QChar letter, QVector<int> fractionals, QString letterID, bool isBold, bool isUnderlined, bool isItalic) :
    letter(letter), fractionalIndexes(fractionals), letterID(letterID),
    isBold(isBold), isUnderlined(isUnderlined), isItalic(isItalic) {}

// Costruttore di copia
Letter::Letter(const Letter& l) {
    this->letter = std::move(l.letter);
    this->letterID = std::move(l.letterID);
    this->fractionalIndexes.append(std::move(l.fractionalIndexes));
}

// Overload operatore di assegnazione
Letter& Letter::operator=(const Letter& source) {
    if(this != &source) {
        this->letter = std::move(source.letter);
        this->letterID = std::move(source.letterID);
        this->fractionalIndexes.erase(this->fractionalIndexes.begin(), this->fractionalIndexes.end());
        this->fractionalIndexes.append(std::move(source.fractionalIndexes));
    }
    return *this;
}

// Overload operatore di movimento
Letter& Letter::operator=(const Letter && source) {
    if(this != &source) {
        this->letter = std::move(source.letter);
        this->letterID = std::move(source.letterID);
        this->fractionalIndexes.erase(this->fractionalIndexes.begin(), this->fractionalIndexes.end());
        this->fractionalIndexes.append(std::move(source.fractionalIndexes));
    }
    return *this;
}

QChar Letter::getValue() {
    return this->letter;
}

QVector<int> Letter::getFractionalIndexes() {
    return this->fractionalIndexes;
}

int Letter::getIndex() {
    return this->fractionalIndexes.at(0);
}

void Letter::setIndex(int index) {
    this->fractionalIndexes.replace(0, index);
}

void Letter::editIndex(int index, int value) {
    this->fractionalIndexes.replace(index, value);
}

QString Letter::getLetterID() {
    return this->letterID;
}

int Letter::getSiteID() {
    int siteID = this->letterID.split("-")[0].toInt();
    return siteID;
}

int Letter::getNumberOfFractionals() {
    return this->fractionalIndexes.size();
}

void Letter::addFractionalDigit(int value) {
    this->fractionalIndexes.append(value);
}

bool Letter::hasSameFractionals(Letter other) {
    QVector<int> otherFractionals = other.getFractionalIndexes();

    if(this->fractionalIndexes.size() == otherFractionals.size()) {
        for(int i=0; i<otherFractionals.size(); i++) {
            if(this->fractionalIndexes.at(i) != otherFractionals.at(i))
                return false;
        }
        return true;
    } else return false;
}

bool Letter::comesFirst(Letter other) {
    bool comesFirst = false;

    if(this->getIndex() < other.getIndex())
        comesFirst = true;
    else {
        if(this->getNumberOfFractionals() < other.getNumberOfFractionals())
            comesFirst = true;
        else if(this->getNumberOfFractionals() == other.getNumberOfFractionals()) {
            for(int i=0; i<this->getNumberOfFractionals(); i++) {
                if(this->fractionalIndexes[i] < other.getFractionalIndexes()[i]) {
                    comesFirst = true;
                    break;
                }
            }
        }
    }
    return comesFirst;
}

void Letter::setBoldBool(bool value){
    this->isBold=value;
}

void Letter::setUnderlinedBool(bool value){
    this->isUnderlined=value;
}

void Letter::setItalicBool(bool value){
    this->isItalic=value;
}

bool Letter::getBoldBool(){
    return this->isBold;
}

bool Letter::getUnderlinedBool(){
    return this->isUnderlined;
}

bool Letter::getItalicBool(){
    return this->isItalic;
}
