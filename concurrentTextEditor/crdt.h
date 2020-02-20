#ifndef CRDT_H
#define CRDT_H

#include <QUuid>
#include <QJsonDocument>
#include <QObject>
#include <QRandomGenerator>

#include "char.h"

class Crdt
{
public:
    Crdt();    
    QString getFileName();
    QString getTextBuffer();
    bool parseCteFile(QJsonDocument unparsedFile);

private:
    QString parseFile(QJsonDocument unparsedFile);
    QUuid createQuuid();

    int base = 32;
    int boundary = 10;

    QString _fileName;
    QUuid _siteID;
    QString _textBuffer;
    QList<Char> _file;
};





#endif // CRDT_H
