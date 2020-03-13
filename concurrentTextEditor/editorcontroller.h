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
    void write();

private:
    Crdt _crdt;

signals:
    void broadcastEditWorker(QString fileName, Char c, EditType editType, int index);

public slots:
    void handleRemoteEdit(const QJsonObject &qjo);

};

#endif // EDITORCONTROLLER_H
