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
    QList<QPair<QString, Format>> getTextBuffer();
    bool parseCteFile(QJsonDocument unparsedFile);
    int findInsertIndex(Char c);
    void handleLocalInsert(QChar val, int index, Format format);
    void handleLocalDelete(int index);
    void handleLocalFormat(int index, Format format);
    Char generateChar(QChar val, int index, Format format);
    QList<Identifier> generatePosBetween(QList<Identifier> posBefore, QList<Identifier> posAfter, QList<Identifier> newPos, int level=0);
    int generateIdBetween(int idBefore, int idAfter, int boundaryStrategy);

    Format getCurrentFormat(int index);

    Char getChar(QJsonObject jsonChar);
    void replaceChar(Char val, int index);

    // Insertion
    void insertChar(Char val, int index);
    void insertText(QChar val, Format format, int index);

    //  Deletion
    void deleteChar(Char val, int index);

    int handleRemoteInsert(const QJsonObject& qjo);
    int handleRemoteDelete(const QJsonObject& qjo);
    int handleRemoteFormat(const QJsonObject& qjo);

    Char _lastChar;
    EditType _lastOperation;
    int retrieveStrategy(int level);
    QUuid getSiteID();
    void updateFileAtIndex(int index, Char c);
    int findIndexByPosition(Char c);

private:
    QList<QPair<QString, Format>> parseFile(QJsonDocument unparsedFile);

    int _base = 32;
    int _boundary = 10;
    int _mult = 2;

    QString _fileName;
    QUuid _siteID;
    QList<QPair<QString, Format>> _textBuffer;
    // File representation
    QList<Char> _file;     
    int _strategy;
};





#endif // CRDT_H
