#ifndef EDITORCONTROLLER_H
#define EDITORCONTROLLER_H

#include <QObject>
#include <QWidget>
#include <QTextEdit>
#include <QKeyEvent>
#include <QJsonDocument>
#include <QtMath>

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

private:
    Crdt _crdt;
    void deleteSelection(int start, int end);
    bool _isPublic;
    QMap<QString, QColor> _usersColor;

signals:
    void broadcastEditWorker(QString fileName, Char c, EditType editType, int index, bool isPublic);

public slots:
    void handleRemoteEdit(const QJsonObject &qjo);

};

#endif // EDITORCONTROLLER_H
