#include "editorcontroller.h"
#include <QKeyEvent>

EditorController::EditorController(QWidget *parent) : QTextEdit(parent)
{

}

void EditorController::keyPressEvent(QKeyEvent *key)
{
    //Enter or return was pressed
    if ( (key->key()==Qt::Key_Enter) || (key->key()==Qt::Key_Return) ) {
        QString cmd = this->toPlainText();
        this->clear();
        return;
    }

    QTextEdit::keyPressEvent(key);
}
