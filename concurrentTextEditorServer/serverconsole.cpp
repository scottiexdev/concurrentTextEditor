#include "serverconsole.h"
#include <QKeyEvent>

ServerConsole::ServerConsole(QWidget *parent)
    : QPlainTextEdit(parent)
{

}

void ServerConsole::keyPressEvent(QKeyEvent *key)
{
    //Enter or return was pressed
    if ( (key->key()==Qt::Key_Enter) || (key->key()==Qt::Key_Return) ) {
        QString cmd = this->toPlainText();
        this->clear();
        emit executeCommand(cmd);
        return;
    }

    QPlainTextEdit::keyPressEvent(key);
}
