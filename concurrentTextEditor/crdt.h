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
    int findInsertIndexInLine(Char c, QList<Char> row);
    void handleLocalInsert(QChar val, QPair<int, int> rowCh, Format format);
    QList<Char> handleLocalDelete(QPair<int,int> startPos, QPair<int,int> endPos);
    void handleLocalFormat(int index, Format format);
    Char generateChar(QChar val, QPair<int, int> rowCh, Format format);
    QList<Identifier> generatePosBetween(QList<Identifier> posBefore, QList<Identifier> posAfter, QList<Identifier> newPos, int level=0);
    int generateIdBetween(int idBefore, int idAfter, int boundaryStrategy);

    Format getCurrentFormat(int index);

    Char getChar(QJsonObject jsonChar);
    void replaceChar(Char val, int index);

    // Insertion
    void insertChar(Char val, QPair<int,int> rowCh);
    void insertText(QChar val, Format format, QPair<int,int> rowCh);

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


    QList<Char> deleteMultipleRows(QPair<int,int> startPos, QPair<int,int> endPos);
    QPair<int, int> findInsertPosition(Char c);
    QPair<int, int> findEndPosition(Char c, QList<Char> lastLine, int totalLines);
    QList<Identifier> findPosBefore(QPair<int, int> rowCh);
    QList<Identifier> findPosAfter(QPair<int, int> rowCh);

    QList<Char> fromReturn(QPair<int, int> rowCh);
    QList<Char> toReturn(QPair<int, int> rowCh);
    QList<Char> lastRowToendPos(QPair<int,int> endPos);

private:
    QList<QPair<QString, Format>> parseFile(QJsonDocument unparsedFile);

    int _base = 32;
    int _boundary = 10;
    int _mult = 2;

    QString _fileName;
    QUuid _siteID;
    QList<QList<QPair<QString, Format>>> _textBuffer;
    // File representation: TO CHANGE, IT MUST BE LIKE A MATRIX
    //QList<Char> _file;
    //QList<QPair<QPair<int, int>, Char>> _file;
    //QVector<QVector<Char>> _file;
    QList<QList<Char>> _file;
    int _strategy;
};





#endif // CRDT_H
