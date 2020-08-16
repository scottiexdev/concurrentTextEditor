#include "editorcontroller.h"
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QClipboard>
#include <QApplication>
#include <QTextBlock>
#include <QTimer>

EditorController::EditorController(QWidget *parent) : QTextEdit(parent)
{    
    _crdt = Crdt();

    //Set margins
    QTextDocument* doc = this->document();
    QTextBlockFormat blockFormat = doc->begin().blockFormat();
    blockFormat.setTopMargin(10);
    blockFormat.setLeftMargin(20);
    blockFormat.setRightMargin(20);
    QTextCursor{doc->end()}.setBlockFormat(blockFormat);
}

void EditorController::keyPressEvent(QKeyEvent *key)
{
    int pressed_key = key->key();
    int cursorPosition = this->textCursor().position();
    int anchor = this->textCursor().anchor();
    int deltaPositions = abs(cursorPosition - anchor);
    int start, end;
    Format currentFormat =_currentFormat;
    //get format
    QTextCharFormat charFormat;

    QString completeFilename = _crdt.getFileName();

    // Filter out not handled ket sequences
    if(!isKeySequenceHandled(key)){
        QTextEdit::keyPressEvent(key);
        return;
    }

    if(_shared == 1)
        completeFilename = _owner + "/" +  _crdt.getFileName();

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

    //ctrl-x handle to avoid "UpArrowBug"
    if(key->matches(QKeySequence::Cut) || pressed_key == Qt::Key_Cut) {
         //cancel the selection (if there is one)
        if(deltaPositions!=0) {
            //Iterate over characters to be removed
            deleteSelection(start, end);
        }
        QTextEdit::keyPressEvent(key);
        return;
    }

    //ctrl-v handler
    if(key->matches(QKeySequence::Paste) || pressed_key == Qt::Key_Paste) {

        QClipboard* clipboard = QApplication::clipboard();
        QString clipText = clipboard->text();
        if(clipText.isNull() || clipText.isEmpty()) {
            return;
        }
        setCurrentFormat(charFormat);
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
    if( (pressed_key >= 0x20 && pressed_key <= 0x0ff && pressed_key != Qt::Key_Control) || pressed_key == Qt::Key_Return || pressed_key == Qt::Key_Tab) {

        setCurrentFormat(charFormat);
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

// Scrive sull'editor il testo parsato
void EditorController::write(){

    QTextCharFormat format;
    //non sarà più una qstring, ma una list di qpairs
    QList<QPair<QString, Format>> textBuffer = _crdt.getTextBuffer();
    //if(_textBuffer.isNull()){
        //throw exception
    //}
    for (auto pair : textBuffer) {
        setFormat(format, pair.second);
        this->textCursor().insertText(pair.first, format);
    }


}


// TODO: Tutte ste eccezioni vanno catchate nell Editor


// Wrappers for crdt methods

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
    bool prova;

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

        case EditType::format:
            //handle format edit
            index = _crdt.handleRemoteFormat(qjo);
            editingCursor = this->textCursor();
            cursorBeforeEdit = this->textCursor();
            editingCursor.setPosition(index);
            this->setTextCursor(editingCursor);
            setFormat(charFormat, format);
            //penso non funzioni, il carattere da prendere è prima del cursore? Se sì, come lo seleziono? Lo seleziono e basta
            //this->textCursor().movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
            //prova = this->textCursor().movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            editingCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            editingCursor.setCharFormat(charFormat);
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
            charFormat.setFontUnderline(false);
            charFormat.setFontItalic(false);
            break;
        case Format::italics:
            charFormat.setFontWeight(QFont::Normal);
            charFormat.setFontUnderline(false);
            charFormat.setFontItalic(true);
            break;
        case Format::underline:
            charFormat.setFontWeight(QFont::Normal);
            charFormat.setFontUnderline(true);
            charFormat.setFontItalic(false);
            break;
        case Format::plain:
            charFormat.setFontWeight(QFont::Normal);
            charFormat.setFontUnderline(false);
            charFormat.setFontItalic(false);
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

void EditorController::changeFormat(int position, int anchor, Format format) {
    if(_currentFormat == format) {
        _currentFormat = Format::plain;
    }
    else {
        _currentFormat = format;
    }
    int start, end;

    QString completeFilename = _crdt.getFileName();

    if(_shared == 1)
        completeFilename = _owner + "/" +  _crdt.getFileName();

    //change format on the editor window
    //QTextCharFormat cursorFormat = this->textCursor().charFormat();
    QTextCharFormat cursorFormat;
    setFormat(cursorFormat,_currentFormat);

        //TODO: come gestisco il plain text? E come tolgo faccio l'annullamento del markup?
    this->textCursor().setCharFormat(cursorFormat); //mergeCharFormat credo che mantenga il charformat precedente, ora provo setcharformat

    //change format on the file (no need to check if deltaPositions != 0 because editor checks if position != anchor
    if(position != anchor) {
        start = anchor > position ? position : anchor;
        end = start == anchor ? position : anchor;

        for(int floatingCursor =  end; floatingCursor > start; floatingCursor--) {
            _crdt.handleLocalFormat(floatingCursor - 1, _currentFormat);
            emit broadcastEditWorker(completeFilename , _crdt._lastChar, _crdt._lastOperation, floatingCursor - 1, _isPublic);
        }
    }
}


void EditorController::setCurrentFormat(QTextCharFormat& charFormat){

    Format currentFormat;

    if(this->textCursor().position() == 0) {
        currentFormat = _currentFormat;
    }
    else {
        currentFormat = _crdt.getCurrentFormat(this->textCursor().position() - 1);
    }

    charFormat.setBackground(Qt::white);
    setFormat(charFormat, _currentFormat); //PROVA FINALE
    this->textCursor().setCharFormat(charFormat);

}


bool EditorController::isKeySequenceHandled(QKeyEvent* key){

    int pressed_key = key->key();
    QString keyText = key->text();
    QFlags<Qt::KeyboardModifier> mod = key->modifiers();

    //conditions that are handled in keyPressEvent
    if(key->matches(QKeySequence::Copy) || key->matches(QKeySequence::Paste) || key->matches(QKeySequence::SelectAll)
            || key->matches(QKeySequence::Cut) || pressed_key == Qt::Key_Control || pressed_key == Qt::Key_Tab
            || pressed_key == Qt::Key_Control || pressed_key == Qt::Key_Return || pressed_key == Qt::Key_Delete
            || pressed_key == Qt::Key_Backspace || pressed_key == Qt::Key_Return || (pressed_key >= 0x20 && pressed_key <= 0x0ff && !mod.testFlag(Qt::ControlModifier))
            || pressed_key == Qt::Key_Paste || pressed_key == Qt::Key_Cut)
        return true;

    return false;
}
