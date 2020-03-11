#ifndef CRDT_H
#define CRDT_H

#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QObject>
#include <QRandomGenerator>
#include <QVector>
#include <QtMath>

#include "Enums.h"
#include "char.h"

class Crdt
{

public:
    Crdt();    
    Crdt(QString siteID);
    QString getFileName();
    QString getTextBuffer();
    bool parseCteFile(QJsonDocument unparsedFile);
    int findInsertIndex(Char c);
    void handleLocalInsert(QChar val, int index);
    void handleLocalDelete(int index);
    Char generateChar(QChar val, int index);
    QList<Identifier> generatePosBetween(QList<Identifier> posBefore, QList<Identifier> posAfter, QList<Identifier> newPos, int level=0);
    int generateIdBetween(int idBefore, int idAfter, int boundaryStrategy);

    // Insertion
    void insertChar(Char val, int index);
    void insertText(QChar val, int index);

    //  Deletion
    void deleteChar(Char val, int index);

    Char _lastChar;
    EditType _lastOperation;
    int retrieveStrategy(int level);
    QUuid getSiteID();
    void updateFileAtIndex(int index, Char c);
    int findIndexByPosition(Char c);

private:
    QString parseFile(QJsonDocument unparsedFile);
    QUuid createQuuid();

    int _base = 32;
    int _boundary = 10;
    int _mult = 2;

    QString _fileName;
    QUuid _siteID;
    QString _textBuffer;
    // File representation
    QList<Char> _file;     
    int _strategy;

};





#endif // CRDT_H
