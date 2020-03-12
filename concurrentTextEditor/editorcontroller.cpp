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

    if( (pressed_key >= 0x20 && pressed_key <= 0x0ff) || (key->key() == Qt::Key_Return)){
        //Init
        int cursorPosition = this->textCursor().position(); // THIS IS WRONG
        int line = this->textCursor().positionInBlock();
        int lines = this->textCursor().blockNumber();
        _crdt.handleLocalInsert(key->text().data()[0], cursorPosition);
        emit broadcastEditWorker(_crdt.getFileName(), _crdt._lastChar, _crdt._lastOperation, cursorPosition);
    }

    if(pressed_key == Qt::Key_Backspace) {
        int cursorPosition = this->textCursor().position()-1;
        if((cursorPosition)!=-1) {
            _crdt.handleLocalDelete(cursorPosition);
            emit broadcastEditWorker(_crdt.getFileName(), _crdt._lastChar, _crdt._lastOperation, cursorPosition);
        }

    }
    //Return "\n"
//    if(key->key() == Qt::Key_Return){
//        int cursorPosition = this->textCursor().position(); // THIS IS WRONG
//        QChar first = key->text().data()[0];
//        QChar second = key->text().data()[1];
//        QChar* data = key->text().data();
//        QString keyText = key->text();
//        _crdt.handleLocalInsert(first, cursorPosition);
//        emit broadcastEditWorker(_crdt.getFileName(), _crdt._lastChar, _crdt._lastOperation);
        //_crdt.handleLocalInsert(second, cursorPosition);
        //emit broadcastEditWorker(_crdt.getFileName(), _crdt._lastChar, _crdt._lastOperation);
//    }

    //Qt::Key_Return (0x1000004)

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

QUuid EditorController::getSiteID() {
    return _crdt.getSiteID();
}

bool EditorController::parseCteFile(QJsonDocument unparsedFile){
    return _crdt.parseCteFile(unparsedFile);
}

void EditorController::handleRemoteEdit(const QJsonObject &qjo) {
    EditType edit = static_cast<EditType>(qjo["editType"].toInt());
    int index;
    QTextCursor pos;
    switch(edit) {
        case EditType::insertion:
            index = _crdt.handleRemoteInsert(qjo);
            pos = this->textCursor();
            this->textCursor().setPosition(index);
            this->textCursor().insertText(_crdt.getChar(qjo)._value);
            this->textCursor().swap(pos);
            break;
        case EditType::deletion:
            index = _crdt.handleRemoteDelete(qjo);
            pos = this->textCursor();
            this->textCursor().setPosition(index);
            this->textCursor().deletePreviousChar();
            this->textCursor().swap(pos);
            break;
        default:
            //handle exception
            break;
    }
}
