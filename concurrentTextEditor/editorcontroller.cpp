#include "editorcontroller.h"
#include <QKeyEvent>
#include <QRandomGenerator>

EditorController::EditorController(QWidget *parent) : QTextEdit(parent)
{
    _crdt = Crdt();
}



void EditorController::keyPressEvent(QKeyEvent *key)
{
    int pressed_key = key->key();


    if( pressed_key >= 0x20 && pressed_key <= 0x0ff){
        //Init
        int cursorPosition = this->textCursor().position();
        _crdt.handleLocalInsert(key->text().data()[0], cursorPosition);
    }

    //    if ( (key->key() == Qt::Key_Enter) || (key->key()==Qt::Key_Return) )


    QTextEdit::keyPressEvent(key);
}

//Scrive sull'editor il testo parsato
void EditorController::write(){

    QString _textBuffer = _crdt.getTextBuffer();
    if(_textBuffer.isNull()){
        //throw exception
    }

    this->append(_textBuffer);
}


//TODO: Tutte ste eccezioni vanno catchate nell Editor


//Wrappers for crdt methods
QString EditorController::getFileName(){

    return _crdt.getFileName();
}

bool EditorController::parseCteFile(QJsonDocument unparsedFile){
    return _crdt.parseCteFile(unparsedFile);
}
