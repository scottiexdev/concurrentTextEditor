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
    QList<QList<QPair<QString, Format>>> getTextBuffer();
    QJsonObject crdtToJson();
    bool parseCteFile(QJsonDocument unparsedFile);
    int findInsertIndexInLine(Char c, QList<Char> row);
    void handleLocalInsert(QChar val, QPair<int, int> rowCh, Format format);
    QList<Char> handleLocalDelete(QPair<int,int> startPos, QPair<int,int> endPos);

    QList<Char> handleLocalFormat(QPair<int,int> startPos, QPair<int,int> endPos, Format format);
    QList<Char> chFormatMultipleRows(QPair<int,int> startPos, QPair<int,int> endPos, Format format);
    QList<Char> chFormatSingleLine(QPair<int,int> startPos, QPair<int,int> endPos, Format format);
    Char generateChar(QChar val, QPair<int, int> rowCh, Format format);
    QList<Identifier> generatePosBetween(QList<Identifier> posBefore, QList<Identifier> posAfter, QList<Identifier> newPos, int level=0);
    int generateIdBetween(int idBefore, int idAfter, int boundaryStrategy);

    Format getCurrentFormat(QPair<int,int> index);

    Char getChar(QJsonObject jsonChar);
    QJsonObject setChar(Char c);
    void replaceChar(Char val, QPair<int,int> rowCh);

    // Insertion
    void insertChar(Char val, QPair<int,int> rowCh);
    void insertText(QChar val, Format format, QPair<int,int> rowCh);

    //  Deletion
    void deleteChar(Char val, int index);
    void removeEmptyRows();
    void removeServerEmptyRows();

    QPair<int,int> handleRemoteInsert(const QJsonObject& qjo);
    QPair<int,int> handleRemoteDelete(const QJsonObject& qjo);
    QPair<int,int> handleRemoteFormat(const QJsonObject& qjo);

    Char _lastChar;
    EditType _lastOperation;
    int retrieveStrategy(int level);
    QUuid getSiteID();
    void updateFileAtPosition(int index, Char c);
    QPair<int,int> findIndexInLine(Char c, QList<Char> row, int pos);

    void mergeRows(int row);
    void mergeServerRows(int row);
    void splitRows(int row, int column);
    void splitRowsBuf(int row, int column);
    QList<Char> deleteSingleLine(QPair<int,int> startPos, QPair<int,int> endPos);
    QList<Char> deleteMultipleRows(QPair<int,int> startPos, QPair<int,int> endPos);
    QList<QPair<QString,Format>> takeMultipleBufRows(QPair<int,int> start, QPair<int,int> end);
    QList<QPair<QString,Format>> takeSingleBufRow(QPair<int,int> startPos, QPair<int,int> endPos);
    QPair<int, int> findPosition(Char c);
    QPair<int, int> findInsertPosition(Char c);
    QPair<int, int> findEndPosition(Char c, QList<Char> lastLine, int totalLines);
    QList<Identifier> findPosBefore(QPair<int, int> rowCh);
    QList<Identifier> findPosAfter(QPair<int, int> rowCh);
    bool containsReturn(QList<Char> chars);

    QList<Char> firstRowToEndLine(QPair<int, int> rowCh);
    QList<Char> lastRowToEndPos(QPair<int,int> endPos);
    QList<QPair<QString,Format>> firstRowToEndLineBuf(QPair<int, int> startPos);
    QList<QPair<QString,Format>> lastRowToEndPosBuf(QPair<int, int> endPos);

    int calcIndex(QPair<int, int> rowCh);
    bool calcBeforePosition(QPair<int,int> start, QPair<int,int> & startBefore);
    bool calcAfterPosition(QPair<int,int> end, QPair<int,int> & endAfter);
    
    void removeChar(Char c, QPair<int,int> index);

private:
    QList<QList<QPair<QString, Format>>> parseFile(QJsonDocument unparsedFile);

    int _base = 64;
    int _boundary = 200;
    int _mult = 2;

    QString _fileName;
    QUuid _siteID;
    QList<QList<QPair<QString, Format>>> _textBuffer;
    // File representation: TO CHANGE, IT MUST BE LIKE A MATRIX
    //QList<Char> _file;
    //QList<QPair<QPair<int, int>, Char>> _file;
    //QVector<QVector<Char>> _file;
    QList<QList<Char>> _file;
    int _strategy = 'plus';
};





#endif // CRDT_H
