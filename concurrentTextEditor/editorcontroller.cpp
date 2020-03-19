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
    Format currentFormat;
    //get format
    if(cursorPosition==0) {
        currentFormat = _currentFormat;
    }
    else {
        currentFormat = _crdt.getCurrentFormat(cursorPosition-1);
    }

    QString completeFilename = _crdt.getFileName();

    if(_shared == 1)
        completeFilename = _owner + "/" +  _crdt.getFileName();

    //clean highlight from remoteEdit


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

    if(key->matches(QKeySequence::SelectAll)) {
        QTextEdit::keyPressEvent(key);
        return;
    }

    QTextCharFormat charFormat = QTextCharFormat();
    charFormat.setBackground(Qt::white);
    setFormat(charFormat, currentFormat);
    this->textCursor().setCharFormat(charFormat);

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
            _crdt.handleLocalInsert(clipText[writingIndex], cursorPosition, currentFormat);
            emit broadcastEditWorker(completeFilename , _crdt._lastChar, _crdt._lastOperation, cursorPosition, _isPublic);
            cursorPosition++;
        }
        this->textCursor().insertText(clipText, charFormat);
        return;
    }

    // Handle Char insert or return
    if( (pressed_key >= 0x20 && pressed_key <= 0x0ff && pressed_key != Qt::Key_Control) || pressed_key == Qt::Key_Return) {

        //cancel the selection (if there is one)
        if(deltaPositions!=0) {
            deleteSelection(start, end);
            cursorPosition=start;
        }

        _crdt.handleLocalInsert(key->text().data()[0], cursorPosition, currentFormat);
        this->textCursor().insertText(key->text().data()[0], charFormat);
        emit broadcastEditWorker(completeFilename , _crdt._lastChar, _crdt._lastOperation, cursorPosition, _isPublic);

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
        emit broadcastEditWorker(completeFilename , _crdt._lastChar, _crdt._lastOperation, cursorPosition -1, _isPublic);
    }


    QTextCursor lastIndex = this->textCursor();
    lastIndex.movePosition(QTextCursor::End);

    // Handle "delete" deletion
    if(pressed_key == Qt::Key_Delete && this->textCursor() != lastIndex && deltaPositions == 0) {

        _crdt.handleLocalDelete(cursorPosition);
        emit broadcastEditWorker(completeFilename , _crdt._lastChar, _crdt._lastOperation, cursorPosition, _isPublic);
    }

    // Let the editor do its thing on current text if no handler is found
    QTextEdit::keyPressEvent(key);
}

void EditorController::deleteSelection(int start, int end) {

    QString completeFilename = _crdt.getFileName();

    if(_shared)
        completeFilename = _owner + "/" + _crdt.getFileName();

    for(int floatingCursor =  end; floatingCursor > start; floatingCursor--) {
        _crdt.handleLocalDelete(floatingCursor - 1);
        emit broadcastEditWorker(completeFilename , _crdt._lastChar, _crdt._lastOperation, floatingCursor - 1, _isPublic);
    }
}

//Scrive sull'editor il testo parsato
void EditorController::write(){

    //non sarà più una qstring, ma una list di qpairs
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

void EditorController::setUserColor(QString user, QColor color) {
    _usersColor.insert(user, color);
}

void EditorController::handleRemoteEdit(const QJsonObject &qjo) {

    int index;
    QString user = qjo["username"].toString();
    QTextCursor editingCursor;
    QTextCursor cursorBeforeEdit;

    QTextCharFormat charFormat;
    charFormat.setBackground(_usersColor[user]);

    EditType edit = static_cast<EditType>(qjo["editType"].toInt());
    Format format = static_cast<Format>(_crdt.getChar(qjo["content"].toObject())._format);

    switch(edit) {

        case EditType::insertion:

            index = _crdt.handleRemoteInsert(qjo);
            editingCursor = this->textCursor();
            cursorBeforeEdit= this->textCursor();
            editingCursor.setPosition(index);
            this->setTextCursor(editingCursor);
            //set format
            setFormat(charFormat, format);
            // Write
            this->textCursor().insertText(QString(_crdt.getChar(qjo["content"].toObject())._value), charFormat);
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

void EditorController::setFormat(QTextCharFormat &charFormat, Format format) {
    switch(format) {
        case Format::bold:
            charFormat.setFontWeight(QFont::Bold);
            break;
        case Format::italics:
            charFormat.setFontItalic(true);
            break;
        case Format::underline:
            charFormat.setFontUnderline(true);
            break;
        case Format::plain:
            //non setto nulla
            break;
        default:
            break;//se entro qua è finita
    }
}

void EditorController::setAccess(bool isPublic){

    _isPublic = isPublic;
}

Crdt EditorController::getCrdt() {
    return _crdt;
}

void EditorController::bold(int position, int anchor) {

    _currentFormat = Format::bold;
    QTextCharFormat bold;
    bold.setFontWeight(QFont::Bold);
    this->textCursor().mergeCharFormat(bold);
    int start, end;
    int deltaPositions = abs(position - anchor);

    QString completeFilename = _crdt.getFileName();

    if(_shared == 1)
        completeFilename = _owner + "/" +  _crdt.getFileName();

    if(deltaPositions != 0){
        start = anchor > position ? position : anchor;
        end = start == anchor ? position : anchor;
    }

}
