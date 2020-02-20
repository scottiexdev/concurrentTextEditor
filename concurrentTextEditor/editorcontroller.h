#ifndef EDITORCONTROLLER_H
#define EDITORCONTROLLER_H

#include <QObject>
#include <QWidget>
#include <QTextEdit>
#include <QKeyEvent>
#include <QJsonDocument>

#include "crdt.h"

class EditorController : public QTextEdit
{
    Q_OBJECT
public:
    explicit EditorController(QWidget *parent = nullptr);
    void keyPressEvent(QKeyEvent *key);
    bool parseCteFile(QJsonDocument unparsedFile);
    QString getFileName();
    void write();

private:
    Crdt _crdt;

};

#endif // EDITORCONTROLLER_H
