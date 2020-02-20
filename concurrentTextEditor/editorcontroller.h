#ifndef EDITORCONTROLLER_H
#define EDITORCONTROLLER_H

#include <QObject>
#include <QWidget>
#include <QTextEdit>
#include <QKeyEvent>

class EditorController : public QTextEdit
{
    Q_OBJECT
public:
    explicit EditorController(QWidget *parent = nullptr);
    void keyPressEvent(QKeyEvent *key);
};

#endif // EDITORCONTROLLER_H
