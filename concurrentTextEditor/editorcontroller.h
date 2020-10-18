#ifndef EDITORCONTROLLER_H
#define EDITORCONTROLLER_H

#include <QObject>
#include <QWidget>
#include <QTextEdit>
#include <QKeyEvent>
#include <QJsonDocument>
#include <QtMath>
#include <QTextDocumentFragment>
#include <QFont>

#include "Enums.h"
#include "crdt.h"

class EditorController : public QTextEdit
{
    Q_OBJECT
public:
    explicit EditorController(QWidget *parent = nullptr);
    void keyPressEvent(QKeyEvent *key);
    bool parseCteFile(QJsonDocument unparsedFile);
    QString getFileName();
    QUuid getSiteID();
    Crdt getCrdt();
    void write();
    void setAccess(bool isPublic);
    void setUserColor(QString user, QColor color);
    void setShared(bool shared) {_shared = shared; }
    void setOwner(QString owner) { _owner = owner; }

    //formatting functions
    void bold(int position, int anchor);
    void italics(int position, int anchor);
    void underline(int position, int anchor);
    void setFormat(QTextCharFormat& charFormat, Format format);
    void changeFormat(QPair<int,int> position, QPair<int,int> anchor, Format format);
    void setCurrentFormat(QTextCharFormat& charFormat);

private:
    Crdt _crdt;
    void deleteSelection(QPair<int,int> start, QPair<int,int> end);
    void takeSelection(QPair<int,int> cursorPosition, QPair<int,int> anchorPosition, QPair<int,int> & startPos, QPair<int,int> & endPos);
    bool _isPublic;
    QMap<QString, QColor> _usersColor;
    bool _shared = false;
    QString _owner = "";
    Format _currentFormat = Format::plain;
    bool isKeySequenceHandled(QKeyEvent* key);

signals:
    void broadcastEditWorker(QString fileName, Char c, EditType editType, QPair<int,int> position, bool isPublic);

public slots:
    void handleRemoteEdit(const QJsonObject &qjo);

};

#endif // EDITORCONTROLLER_H
