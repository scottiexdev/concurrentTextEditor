#include "editorcontroller.h"
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QClipboard>
#include <QApplication>

EditorController::EditorController(QWidget *parent) : QTextEdit(parent)
{    
    _crdt = Crdt();    
}



void EditorController::keyPressEvent(QKeyEvent *key)
{
    int pressed_key = key->key();
    int cursorPosition = this->textCursor().position();
    int anchor = this->textCursor().anchor();
    int deltaPositions = abs(cursorPosition - anchor);
    int start, end;

    //clean highlight from remoteEdit
    QTextCharFormat highlight = QTextCharFormat();
    highlight.setBackground(Qt::white);
    this->textCursor().setCharFormat(highlight);

    //take selection if there is one
    if(deltaPositions != 0){
        start = anchor > cursorPosition ? cursorPosition : anchor;
        end = start == anchor ? cursorPosition : anchor;
    }

    //ctrl-c handler to avoid "HeartBug"
    if(key->matches(QKeySequence::Copy) || pressed_key == Qt::Key_Control){
        QTextEdit::keyPressEvent(key);
        return;
    }

    //ctrl-x handle to avoid "UpArrowBug"
    if(key->matches(QKeySequence::Cut)) {
         //cancel the selection (if there is one)
        if(deltaPositions!=0) {
            //Iterate over characters to be removed
            deleteSelection(start, end);
        }
        QTextEdit::keyPressEvent(key);
        return;
    }

    //ctrl-v handler
    if(key->matches(QKeySequence::Paste)){

        QClipboard* clipboard = QApplication::clipboard();
        QString clipText = clipboard->text();

        if(deltaPositions!=0) {
            deleteSelection(start, end);
            //for insert we need to set the position to start, either it's out of range
            cursorPosition=start;
        }

        // Write clipboard text into crdt and broadcast edit        
        for(int writingIndex = 0; writingIndex <  clipText.length(); writingIndex++){
            _crdt.handleLocalInsert(clipText[writingIndex], cursorPosition);
            emit broadcastEditWorker(_crdt.getFileName(), _crdt._lastChar, _crdt._lastOperation, cursorPosition);
            cursorPosition++;
        }
        this->textCursor().insertText(clipText,highlight);
        return;
    }

    // Handle Char insert or return
    if( (pressed_key >= 0x20 && pressed_key <= 0x0ff) || pressed_key == Qt::Key_Return) {

        //cancel the selection (if there is one)
        if(deltaPositions!=0) {
            deleteSelection(start, end);
            cursorPosition=start;
        }

        _crdt.handleLocalInsert(key->text().data()[0], cursorPosition);
        this->textCursor().insertText(key->text().data()[0],highlight);
        emit broadcastEditWorker(_crdt.getFileName(), _crdt._lastChar, _crdt._lastOperation, cursorPosition);
        return;
    }


    //maybe the next two are refactorable
    // Handle selection deletion with backspace or delete key
    if((pressed_key == Qt::Key_Backspace || pressed_key == Qt::Key_Delete) && deltaPositions != 0) {
        deleteSelection(start, end);
    }

    // Handle backspace deletion
    if(pressed_key == Qt::Key_Backspace && (cursorPosition -1) != -1 && deltaPositions == 0) {

        _crdt.handleLocalDelete(cursorPosition -1);
        emit broadcastEditWorker(_crdt.getFileName(), _crdt._lastChar, _crdt._lastOperation, cursorPosition -1);
    }


    QTextCursor lastIndex = this->textCursor();
    lastIndex.movePosition(QTextCursor::End);

    // Handle "delete" deletion + TODO: capire se sono alla fine di un testo, nel caso non posso fare canc
    if(pressed_key == Qt::Key_Delete && this->textCursor() != lastIndex && deltaPositions == 0) {

        _crdt.handleLocalDelete(cursorPosition);
        emit broadcastEditWorker(_crdt.getFileName(), _crdt._lastChar, _crdt._lastOperation, cursorPosition);
    }

    // Let the editor do its thing on current text if no handler is found
    QTextEdit::keyPressEvent(key);
}

void EditorController::deleteSelection(int start, int end) {
    for(int floatingCursor =  end; floatingCursor > start; floatingCursor--) {
        _crdt.handleLocalDelete(floatingCursor - 1);
        emit broadcastEditWorker(_crdt.getFileName(), _crdt._lastChar, _crdt._lastOperation, floatingCursor - 1);
    }
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

    int index;
    QTextCursor editingCursor;
    QTextCursor cursorBeforeEdit;

    QTextCharFormat highlight = QTextCharFormat();
    highlight.setBackground(Qt::red);

    EditType edit = static_cast<EditType>(qjo["editType"].toInt());

    switch(edit) {

        case EditType::insertion:

            index = _crdt.handleRemoteInsert(qjo);
            editingCursor = this->textCursor();
            cursorBeforeEdit= this->textCursor();
            editingCursor.setPosition(index);
            this->setTextCursor(editingCursor);
            // Write
            this->textCursor().insertText(QString(_crdt.getChar(qjo["content"].toObject())._value), highlight);
            // Set cursor back to original position (before editing)
            this->setTextCursor(cursorBeforeEdit);

            break;

        case EditType::deletion:

            index = _crdt.handleRemoteDelete(qjo);
            editingCursor = this->textCursor();
            cursorBeforeEdit = this->textCursor();
            editingCursor.setPosition(index + 1);
            this->setTextCursor(editingCursor);
            this->textCursor().deletePreviousChar();
            this->setTextCursor(cursorBeforeEdit);

            break;

        default:
            //handle exception
            break;
    }
}
