#include "crdt.h"


Crdt::Crdt()
{

     _siteID = createQuuid();
}

QUuid Crdt::createQuuid(){

    auto generator = QRandomGenerator();
    QVector<int> seeds;

    for(int i = 0; i < 11; i++){
        int seed = generator.bounded(10000);
        seeds.push_front(seed);
    }

    return QUuid(seeds[0],seeds[1],seeds[2],seeds[3],seeds[4],seeds[5],seeds[6],seeds[7],seeds[8],seeds[9],seeds[10]);
}

bool Crdt::parseCteFile(QJsonDocument unparsedFile){
    _textBuffer = parseFile(unparsedFile);
    if(_textBuffer.isNull())
        return false;
    return true;
}


QString Crdt::parseFile(QJsonDocument unparsedFile){

    //costruzione della lista di Char
    QString buf;
    QJsonObject obj = unparsedFile.object();

    _fileName = obj["requestedFiles"].toString();

    QString fileContentString = unparsedFile["fileContent"].toString(); //string con tutto il content - QJsonValue

    QByteArray strBytes = fileContentString.toUtf8();

    QJsonDocument contentDoc = QJsonDocument::fromJson(strBytes);

    QJsonObject fileContentObj = contentDoc.object();

    QJsonArray arrayBuf = fileContentObj["content"].toArray();

    if(arrayBuf.empty())
        return "";

    foreach (const QJsonValue & tmpChar, arrayBuf) {
        QJsonObject charObject = tmpChar.toObject();
        QChar val = charObject["value"].toString()[0];
        QUuid siteID = charObject["siteID"].toString();
        int counter = charObject["counter"].toInt();

        //Now parse positions: conversion to array is needed, but to object first

        QJsonArray identifiers = charObject["position"].toArray();

        QList<Identifier> positions;
        foreach (const QJsonValue &tmpID, identifiers) {
            QJsonObject ID = tmpID.toObject();
            int digit = ID["digit"].toInt();
            QUuid oldSiteID = ID["siteID"].toString();
            Identifier identifier(digit,oldSiteID);
            positions.append(identifier);
        }

        Char c(val,counter,siteID,positions);
        _file.append(c);
    }

    //lettura dei caratteri della QList<Char> e scrittura nel buffer
    _textBuffer.clear();

    foreach(const Char &tmpC, _file) {
        int index = findInsertIndex(tmpC);
        _textBuffer.insert(index,tmpC._value);
    }

    return _textBuffer;
}

void Crdt::handleLocalInsert(QChar val, int index) {
    Char c = generateChar(val, index);
    insertChar(c, index);
    insertText(c._value, index);

    _lastChar = c;
    _lastOperation = EditType::insertion;
}

void Crdt::insertChar(Char c, int index) {

    _file.insert(index, c);
}

void Crdt::insertText(QChar val, int index) {

    _textBuffer = _textBuffer.insert(index, val);
}

Char Crdt::generateChar(QChar val, int index) {
    QList<Identifier> posBefore;
    QList<Identifier> posAfter;
    QList<Identifier> newPos;
    if(index-1 >= 0)
        posBefore = _file.at(index - 1)._position;
    if(index < _file.length())
        posAfter = _file.at(index)._position;
    newPos = generatePosBetween(posBefore, posAfter, newPos);
    //TODO: version counter per globality
    //const localCounter = this.vector.localVersion.counter;

    return Char(val, 0/*to implement*/, _siteID, newPos);
}

QList<Identifier> Crdt::generatePosBetween(QList<Identifier> posBefore, QList<Identifier> posAfter, QList<Identifier> newPos, int level) {

    int base = (int)qPow(_mult, level) * _base;
    int boundaryStrategy = qFloor(QRandomGenerator().bounded(0xffffffff)) == 0 ? '+' : '-';
    Identifier id1;
    Identifier id2;
    QList<Identifier> emptyAfter;

    if(posBefore.isEmpty())
        id1 = Identifier(0, _siteID);
    else
        id1 = posBefore[0];

    if(posAfter.isEmpty())
        id2 = Identifier(base, _siteID);
    else
        id2 = posAfter[0];

    if (id2._digit - id1._digit > 1) {
      int newDigit = generateIdBetween(id1._digit, id2._digit, boundaryStrategy);
      newPos.append(Identifier(newDigit, _siteID));
      return newPos;

    }
    else if (id2._digit - id1._digit == 1) {
      newPos.append(id1);
      return generatePosBetween(posBefore.mid(1), emptyAfter, newPos, level+1);
    }
    else if (id1._digit == id2._digit) {
      if (id1._siteID < id2._siteID) {
        newPos.append(id1);
        return generatePosBetween(posBefore.mid(1), emptyAfter, newPos, level+1);
      } else if (id1._siteID == id2._siteID) {
        newPos.append(id1);
        return generatePosBetween(posBefore.mid(1), posAfter.mid(1), newPos, level+1);
      } else {
        //implement exception
        //throw new Error("Fix Position Sorting");
      }
    }
}

int Crdt::generateIdBetween(int min, int max, int boundaryStrategy) {

    if ((max - min) < _boundary) {
      min = min + 1;
    }
    else {
        if (boundaryStrategy == '-')
            min = max - _boundary;
        else {
            min = min + 1;
            max = min + _boundary;
        }
    }

    return qFloor(QRandomGenerator().bounded((double)1) * (max - min)) + min;
}

int Crdt::findInsertIndex(Char c) {

    int left = 0;
    int right = _file.length() - 1;
    int mid, compareNum;

    if (_file.length() == 0 || c.compareTo(_file.at(left)) < 0) {
      return left;
    } else if (c.compareTo(_file.at(right)) > 0) {
      return _file.length();
    }

    while (left + 1 < right) {
      mid = qFloor(left + (right - left) / 2);
      compareNum = c.compareTo(_file.at(mid));

      if (compareNum == 0) {
        return mid;
      } else if (compareNum > 0) {
        left = mid;
      } else {
        right = mid;
      }
    }

    return c.compareTo(_file.at(left)) == 0 ? left : right;

}



QString Crdt::getFileName(){

    if(_fileName.isNull() || _fileName.isEmpty()){
        //throw exception
    }

    return _fileName;
}

QString Crdt::getTextBuffer(){

    if(_textBuffer.isNull() || _textBuffer.isEmpty()){
        //throw exception
    }
    return _textBuffer;
}
