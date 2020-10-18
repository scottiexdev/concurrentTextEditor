#include "crdt.h"


Crdt::Crdt()
{
     _siteID = QUuid::createUuid();
}

Crdt::Crdt(QString siteID) {
    _siteID = QUuid(siteID);
}

bool Crdt::parseCteFile(QJsonDocument unparsedFile){
    _textBuffer = parseFile(unparsedFile);
    if(_textBuffer.isEmpty())
        return false;
    return true;
}


QList<QList<QPair<QString, Format>>> Crdt::parseFile(QJsonDocument unparsedFile){

    //costruzione della lista di Char
    _file.clear();
    QString buf;
    QJsonObject obj = unparsedFile.object();
    QString fileName = obj["requestedFiles"].toString();

    if(fileName.split("/").size() == 2) {
        _fileName = fileName.split("/")[1];
    } else {
        _fileName = fileName.split("/")[0];
    }

    QString fileContentString = unparsedFile["fileContent"].toString(); //string con tutto il content - QJsonValue

    QByteArray strBytes = fileContentString.toUtf8();

    QJsonDocument contentDoc = QJsonDocument::fromJson(strBytes);

    QJsonObject fileContentObj = contentDoc.object();

    QJsonArray arrayBuf = fileContentObj["content"].toArray();

    if(arrayBuf.empty())
        return QList<QList<QPair<QString, Format>>>();

    // Clear buffers
    _textBuffer.clear();
    _file.clear();

    foreach (const QJsonValue & tmpChar, arrayBuf) {
        Char c = getChar(tmpChar.toObject());

        //Find index for new char and insert it into _file Struct and _textBuffer
        //NOT NECESSARY ANYMORE, THE FILE IS WRITTEN IN ORDER
        QPair<int,int> rowCh = findInsertPosition(c);
        insertChar(c, rowCh);
        insertText(c._value, c._format, rowCh);
//        _file.append(c);
//        _textBuffer.append(QPair<QString, Format>(c._value, c._format));
    }

    return _textBuffer;
}

void Crdt::handleLocalInsert(QChar val, QPair<int, int> rowCh, Format format) {

    Char c = generateChar(val, rowCh, format);
    insertChar(c, rowCh);
    insertText(c._value, c._format, rowCh);

    _lastChar = c;
    _lastOperation = EditType::insertion;
}

QList<Char> Crdt::handleLocalDelete(QPair<int,int> startPos, QPair<int,int> endPos) {
    QList<Char> chars;
    bool newRowRemoved = false;
    if(startPos.first != endPos.first) { //compare rows
        // delete chars on first line from startPos.ch to end of line
        newRowRemoved = true;
        chars = deleteMultipleRows(startPos, endPos);
    } else {
        chars = deleteSingleLine(startPos, endPos);

<<<<<<< HEAD
        if(chars[chars.length()-1]._value == '\n')
            newRowRemoved = true;
        }

    if(newRowRemoved && !_file[startPos.first + 1].isEmpty()) { //maybe check if last line
        mergeRows(startPos.first);
    }
=======
        foreach(Char item, chars) {
            if(item._value == '\n') newRowRemoved = true;
        }
        // ^ inserire un break dentro l'if oppure no?
    }

    // eliminare i caratteri da _file & _textbuffer (??)



>>>>>>> f90a0059ee3d8448e5667b40220277e47aded611
}



QList<Char> Crdt::deleteMultipleRows(QPair<int,int> startPos, QPair<int,int> endPos) {
    QList<Char> chars = firstRowToEndLine(startPos);
    int row;
    for(row = startPos.first + 1; row < endPos.first; row++) {
        chars.append(_file[row]);
        _file.removeAt(row);
        _textBuffer.removeAt(row);
    }

    if(!_file[endPos.first].isEmpty()) {
        chars.append(lastRowToendPos(endPos)); //take the chars of the last selected row til endPos.ch
        deleteSingleLine(QPair<int,int>(endPos.first,0), endPos);
    }
    return chars;
}

QList<Char> Crdt::deleteSingleLine(QPair<int, int> startPos, QPair<int, int> endPos) {
    int charNum = endPos.second - startPos.second;
    for(int i=startPos.second; i< charNum; i++) {
        _file[startPos.first].removeAt(i);
        _textBuffer[startPos.first].removeAt(i);
    }
}

void Crdt::mergeRows(int row) {
    QList<Char> rowAfter = _file.takeAt(row+1);
    _file[row].append(rowAfter);
}
//QList<Char> Crdt::handleLocalDelete(QPair<int,int> startPos, QPair<int,int> endPos) {

//    Char c = _file.takeAt(index);
//    _textBuffer.removeAt(index);

//    _lastChar = c;
//    _lastOperation = EditType::deletion;
//}

QList<Char> Crdt::firstRowToEndLine(QPair<int, int> startPos){
    // TODO: capire se va bene così oppure cambiarla concettualmente.
    QList<Char> buff = _file[startPos.first];
    QList<Char> res(buff.mid(startPos.second));
    return res;
}

QList<Char> Crdt::lastRowToendPos(QPair<int, int> endPos){
    QList<Char> buff = _file[endPos.first];
    QList<Char> res(buff.mid(0, endPos.second));
    return res;
}

QList<Char> Crdt::handleLocalFormat(QPair<int,int> startPos, QPair<int,int> endPos, Format format) {

    QList<Char> chars;

    if(startPos.first != endPos.first) { //compare rows
        // delete chars on first line from startPos.ch to end of line
        chars = chFormatMultipleRows(startPos, endPos, format);
    } else {
        chars = chFormatSingleLine(startPos, endPos, format);
    }

    return chars;
//    Char c = _file[rowCh.first][rowCh.second];
//    c._format = format;
//    replaceChar(c, rowCh);

//    _lastChar = c;
//    _lastOperation = EditType::format;
}

QList<Char> Crdt::chFormatMultipleRows(QPair<int,int> startPos, QPair<int,int> endPos, Format format) {
    QList<Char> chars = firstRowToEndLine(startPos);
    int row;
    for(row = startPos.first + 1; row < endPos.first; row++) {
        chars.append(_file[row]);
    }

    if(!_file[endPos.first].isEmpty()) {
        chars.append(lastRowToendPos(endPos)); //take the chars of the last selected row til endPos.ch
    }
    return chars;
}

QList<Char> Crdt::chFormatSingleLine(QPair<int, int> startPos, QPair<int, int> endPos, Format format) {
    return _file[startPos.first].mid(startPos.second,endPos.second-startPos.second);
}

void Crdt::replaceChar(Char val, QPair<int,int> rowCh) {

    _file[rowCh.first][rowCh.second] = val;
}

void Crdt::insertChar(Char c, QPair<int,int> rowCh) {

    int row = rowCh.first;
    int ch = rowCh.second;

    if(row == _file.length()) {
        _file[row].append(Char());
    }

    if(c._value == '\n') {
        QList<Char> rowAfter = firstRowToEndLine(rowCh); //take all characters after the \n
        if(rowAfter.length() == 0) {
            _file[row].insert(ch, c);
        } else {
            _file[row].append(c);
            _file.insert(row+1, rowAfter); //if there are characters in next lines?
        }
    } else {
        _file[row].insert(ch, c);
    }
}

void Crdt::insertText(QChar val, Format format, QPair<int,int> rowCh) {

    _textBuffer[rowCh.first].insert(rowCh.second, QPair<QString,Format>(val,format));
}

Char Crdt::generateChar(QChar val, QPair<int, int> rowCh, Format format) {

    QList<Identifier> posBefore = findPosBefore(rowCh);
    QList<Identifier> posAfter = findPosAfter(rowCh);
    QList<Identifier> newPos = generatePosBetween(posBefore, posAfter, newPos);
//    if(index-1 >= 0)
//        posBefore = _file.at(index - 1)._position;
//    if(index < _file.length())
//        posAfter = _file.at(index)._position;

    //TODO: version counter per globality
    //const localCounter = this.vector.localVersion.counter;

    return Char(val, 0, _siteID, newPos, format);
}

QList<Identifier> Crdt::findPosBefore(QPair<int, int> rowCh) {
    int row = rowCh.first;
    int ch = rowCh.second;

    if(ch == 0 && row == 0) {
        return QList<Identifier>();
    } else if (ch == 0 && row != 0) { //first char of the row, but not the first one
        row -= 1;
        ch = _file[row].length();
    }

    return _file[row][ch-1]._position;
}

QList<Identifier> Crdt::findPosAfter(QPair<int, int> rowCh) {
    int row = rowCh.first;
    int ch = rowCh.second;
    int numRows = _file.length();//in theory this is the number of rows, debug needed
    int numChars = _file[row].length();

    if((row == numRows-1) && (ch == numChars)) {
        return QList<Identifier>();
    } else if ((row < numRows-1) && (ch == numChars)) {
        row += 1;
        ch = 0;
    } else if ((row > numRows -1 && ch == 0)) {
        return QList<Identifier>();
    }

    return _file[row][ch]._position;
}

int Crdt::retrieveStrategy(int level) {
  int strategy;

  switch (this->_strategy) {
    case 'plus':
      strategy = '+';
      break;
    case 'minus':
      strategy = '-';
      break;
    case 'random':
      //strategy = Math.round(Math.random()) === 0 ? '+' : '-';
      break;
    case 'every2nd':
      strategy = ((level+1) % 2) == 0 ? '-' : '+';
      break;
    case 'every3rd':
      strategy = ((level+1) % 3) == 0 ? '-' : '+';
      break;
    default:
      strategy = ((level+1) % 2) == 0 ? '-' : '+';
      break;
  }
  return strategy;
}

QList<Identifier> Crdt::generatePosBetween(QList<Identifier> posBefore, QList<Identifier> posAfter, QList<Identifier> newPos, int level) {

    int base = (int)qPow(_mult, level) * _base;
    int boundaryStrategy = retrieveStrategy(level);
    Identifier id1;
    Identifier id2;
    QList<Identifier> emptyAfter;

    if(posBefore.isEmpty())
        id1 = Identifier(0, _siteID);
    else
        id1 = posBefore[0];

    if(posAfter.isEmpty())
        id2 = Identifier(base, _siteID);
    else
        id2 = posAfter[0];

    if (id2._digit - id1._digit > 1) {
      int newDigit = generateIdBetween(id1._digit, id2._digit, boundaryStrategy);
      newPos.append(Identifier(newDigit, _siteID));
      return newPos;

    }
    else if (id2._digit - id1._digit == 1) {
      newPos.append(id1);
      return generatePosBetween(posBefore.mid(1), emptyAfter, newPos, level+1);
    }
    else if (id1._digit == id2._digit) {
      if (id1._siteID < id2._siteID) {
        newPos.append(id1);
        return generatePosBetween(posBefore.mid(1), emptyAfter, newPos, level+1);
      } else if (id1._siteID == id2._siteID) {
        newPos.append(id1);
        return generatePosBetween(posBefore.mid(1), posAfter.mid(1), newPos, level+1);
      }
    }

    QList<Identifier> empty;
    return empty;
}

int Crdt::generateIdBetween(int min, int max, int boundaryStrategy) {

    if ((max - min) < _boundary) {
      min = min + 1;
    }
    else {
        if (boundaryStrategy == '-')
            min = max - _boundary;
        else {
            min = min + 1;
            max = min + _boundary;
        }
    }

    return qFloor(QRandomGenerator().bounded((double)1) * (max - min)) + min;
}

QPair<int, int> Crdt::findInsertPosition(Char c) {
    int minRow = 0;
    int totalRows = _file.length();
    int maxRow = totalRows - 1;
    QList<Char> lastRow = _file[maxRow];
    int midRow;
    QList<Char> currentRow, minCurrentRow, maxCurrentRow;
    Char minLastChar, maxLastChar;
    int charIndex;

    if(_file.isEmpty() || c.compareTo(_file[0][0]) <= 0) {
        return QPair<int, int>(0,0);
    }

    Char lastC = lastRow[lastRow.length() - 1];

    if(c.compareTo(lastC) > 0) {
        return findEndPosition(lastC, lastRow, totalRows);
    }

    //binary search
    while(minRow + 1 < maxRow) {
        midRow = qFloor(minRow+(maxRow-minRow)/2);
        currentRow = _file[midRow];
        lastC = currentRow[currentRow.length()-1];

        if(c.compareTo(lastC) == 0) {
            return QPair<int,int>(midRow,currentRow.length()-1);
        } else if (c.compareTo(lastC) < 0) {
            maxRow = midRow;
        } else {
            minRow = midRow;
        }
    }

    //check between min and max line
    minCurrentRow = _file[minRow];
    minLastChar = minCurrentRow[minCurrentRow.length()-1];
    minCurrentRow = _file[minRow];
    maxLastChar = maxCurrentRow[maxCurrentRow.length()-1];

    if(c.compareTo(minLastChar) <= 0) {
        charIndex = findInsertIndexInLine(c, minCurrentRow);
        return QPair<int,int>(minRow,charIndex);
    } else {
        charIndex = findInsertIndexInLine(c, maxCurrentRow);
        return QPair<int,int>(maxRow,charIndex);
    }
}

QPair<int, int> Crdt::findEndPosition(Char c, QList<Char> lastRow, int totalLines) {
    if(c._value == '\n') {
        return QPair<int,int>(totalLines,0);
    } else {
        return QPair<int,int>(totalLines-1, lastRow.length());
    }
}

int Crdt::findInsertIndexInLine(Char c, QList<Char> row) {

    int left = 0;
    int right = _file.length() - 1;
    int mid, compareNum;

    if (row.length() == 0 || c.compareTo(row[left]) < 0) {
      return left;
    } else if (c.compareTo(row[right]) > 0) {
      return _file.length();
    }

    while (left + 1 < right) {
      mid = qFloor(left + (right - left) / 2);
      compareNum = c.compareTo(row[mid]);

      if (compareNum == 0) {
        return mid;
      } else if (compareNum > 0) {
        left = mid;
      } else {
        right = mid;
      }
    }

    return c.compareTo(row[left]) == 0 ? left : right;

}

//useless, never called
//void Crdt::updateFileAtPosition(int index, Char c){
//    _file.insert(index, c);
//}


QString Crdt::getFileName(){
    return _fileName;
}

QList<QList<QPair<QString, Format>>> Crdt::getTextBuffer(){
    return _textBuffer;
}

QUuid Crdt::getSiteID() {
    return _siteID;
}


int Crdt::findIndexByPosition(Char c){

    int left = 0;
    int right = _file.length()- 1;
    int mid, compareNum;

    if (_file.length() == 0) {
      return -1;
    }

    while (left + 1 < right) {
      mid = qFloor(left + (right - left) / 2);
      compareNum = c.compareTo(_file[mid]);

      if (compareNum == 0) {
        return mid;
      }
      else if (compareNum > 0) {
        left = mid;
      }
      else {
        right = mid;
      }
    }

    if (c.compareTo(_file[left]) == 0) {
      return left;
    }
    else if (c.compareTo(_file[right]) == 0) {
      return right;
    }
    else {
        return -1;
    }
}

void Crdt::deleteChar(Char val, int index){
    _file.removeAt(index);
}

int Crdt::handleRemoteDelete(const QJsonObject &qjo) {

    Char c = getChar(qjo["content"].toObject());
    int index = findIndexByPosition(c);
    _file.removeAt(index);
    _textBuffer.removeAt(index);

    return index;
}

QPair<int,int> Crdt::handleRemoteInsert(const QJsonObject &qjo) {

    Char c = getChar(qjo["content"].toObject());
    QPair<int,int> position = findInsertPosition(c);
    insertChar(c, position);
    insertText(c._value,c._format,position);
    //_textBuffer.insert(position, QPair<QString,Format>(c._value,c._format));

    return position;
}

int Crdt::handleRemoteFormat(const QJsonObject &qjo) {

    Char c = getChar(qjo["content"].toObject());
    int index = findIndexByPosition(c);
    _file.replace(index, c);
    _textBuffer.replace(index, QPair<QString,Format>(c._value,c._format));
    //no need to modify textbuffer
    return index;
}

Char Crdt::getChar(QJsonObject jsonChar ){

    // Estrazione di Char da newChar JSonObject
    QChar val = jsonChar["value"].toString()[0];
    Format format = static_cast<Format>(jsonChar["format"].toInt());
    QUuid siteID = jsonChar["siteID"].toString();
    int counter = jsonChar["counter"].toInt();
    QJsonArray identifiers = jsonChar["position"].toArray();
    QPair<int, int> rowCh = QPair<int, int>(jsonChar["row"].toInt(), jsonChar["ch"].toInt());
    QList<Identifier> positions;

    foreach (const QJsonValue &tmpID, identifiers) {
        QJsonObject ID = tmpID.toObject();
        int digit = ID["digit"].toInt();
        QUuid oldSiteID = ID["siteID"].toString();
        Identifier identifier(digit,oldSiteID);
        positions.append(identifier);
    }

    return Char(val,counter,siteID,positions,format);
}


Format Crdt::getCurrentFormat(int index) {
    return _file.at(index)._format;
}

int Crdt::calcIndex(QPair<int, int> rowCh) {
    int index = 0;
    int row;
    for(row = 0; row < rowCh.first; row++)
        index += _file[row].length();
    index += rowCh.second;
    return index;
}

void Crdt::calcBeforePosition(QPair<int,int> start, QPair<int,int> & startBefore) {
    if(start.second - 1 == -1) {
        startBefore.first = start.first - 1;
        startBefore.second = _file[startBefore.first].length()-1;
    }
    else {
        startBefore.first = start.first;
        startBefore.second = start.second - 1;
    }
}
