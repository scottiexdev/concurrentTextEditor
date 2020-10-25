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
        QPair<int,int> rowCh = findInsertPosition(c);
        insertChar(c, rowCh);
        insertText(c._value, c._format, rowCh);
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

        if(chars[chars.length()-1]._value == '\r' || chars[chars.length()-1]._value == '\n')
            newRowRemoved = true;
        }

    if(newRowRemoved && startPos.first+1 != _file.length()) { //maybe check if last line
        mergeRows(startPos.first);
    }

    return chars;
}



QList<Char> Crdt::deleteMultipleRows(QPair<int,int> startPos, QPair<int,int> endPos) {
    QPair<int,int> lastRow(startPos.first+1, endPos.second);
    QList<Char> chars = firstRowToEndLine(startPos);
    deleteSingleLine(startPos, QPair<int,int>(startPos.first, _file[startPos.first].length()));
    int row=0;
    int rowsToDelete = endPos.first - startPos.first - 1;
    while(row != rowsToDelete) {
        chars.append(_file.takeAt(startPos.first+1));
        _textBuffer.takeAt(startPos.first+1);
        row++;
    }

    if(!_file[startPos.first+1].isEmpty()) {
        chars.append(lastRowToEndPos(lastRow)); //take the chars of the last selected row til endPos.ch
        deleteSingleLine(QPair<int,int>(lastRow.first,0), lastRow);
        if(_file[startPos.first].isEmpty()){
                _file.removeAt(startPos.first);
        }
    }

    return chars;
}

QList<Char> Crdt::deleteSingleLine(QPair<int, int> startPos, QPair<int, int> endPos) {
    QList<Char> toremove;
    Char tmp;
    int charNum = (endPos.second - startPos.second);
    int i = startPos.second;
    int currChar = 0;

    if(charNum == 0) {
        tmp = _file[startPos.first].takeAt(startPos.second);
        toremove.append(tmp);
        _textBuffer[startPos.first].takeAt(startPos.second);
    } else {
        while(currChar != charNum) {
            tmp = _file[startPos.first].takeAt(i);
            toremove.append(tmp);
            _textBuffer[startPos.first].takeAt(i);
            currChar++;
        }
    }

    return toremove;
}

void Crdt::mergeRows(int row, bool client) {

    QList<Char> rowAfterFile = _file.takeAt(row+1);
    _file[row].append(rowAfterFile);

    if(client){
        QList<QPair<QString,Format>> rowAfterBuf = _textBuffer.takeAt(row+1);
        _textBuffer[row].append(rowAfterBuf);
    }
}

QList<Char> Crdt::firstRowToEndLine(QPair<int, int> startPos){

    QList<Char> buff = _file[startPos.first];
    QList<Char> res(buff.mid(startPos.second));

    return res;
}

QList<Char> Crdt::lastRowToEndPos(QPair<int, int> endPos){

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
}

QList<Char> Crdt::chFormatMultipleRows(QPair<int,int> startPos, QPair<int,int> endPos, Format format) {

    QList<Char> chars;
    int row;

    chFormatSingleLine(startPos, QPair<int,int>(startPos.first, _file[startPos.first].length()), format);
    chars = firstRowToEndLine(startPos);
    for(row = startPos.first + 1; row < endPos.first; row++) {
        chFormatSingleLine(QPair<int,int>(row,0), QPair<int,int>(row, _file[row].length()), format);
        chars.append(_file[row]);
    }

    if(!_file[endPos.first].isEmpty()) {
        chFormatSingleLine(QPair<int,int>(endPos.first,0), endPos, format);
        chars.append(lastRowToEndPos(endPos)); //take the chars of the last selected row til endPos.ch
    }

    return chars;
}

QList<Char> Crdt::chFormatSingleLine(QPair<int, int> startPos, QPair<int, int> endPos, Format format) {
    for(int i=startPos.second; i<endPos.second; i++) {
        Char c = _file[startPos.first][i];
        c._format = format;
        replaceChar(c,QPair<int,int>(startPos.first,i));
    }

    return _file[startPos.first].mid(startPos.second,endPos.second-startPos.second);
}

void Crdt::replaceChar(Char val, QPair<int,int> rowCh) {

    _file[rowCh.first][rowCh.second] = val;
}

void Crdt::insertChar(Char c, QPair<int,int> rowCh) {

    int row = rowCh.first;
    int ch = rowCh.second;

    if(row >= _file.length()) { //means that it is a new line
        _file.append(QList<Char>());
    }

    if(c._value == '\n' || c._value == '\r') {
        QList<Char> rowAfter = firstRowToEndLine(rowCh); //take all characters after the \n
        if(rowAfter.length() == 0) {
            _file[row].insert(ch, c);
            _file.insert(row+1, QList<Char>());
        } else {
            _file[row].insert(ch, c);
            splitRows(row, ch);
            _file.insert(row+1, rowAfter); //if there are characters in next lines? Scala perché è una lista
        }
    }
    else {
        _file[row].insert(ch, c);
    }
}

void Crdt::splitRows(int row, int column) {

    int i = column+1;

    while(i != _file[row].length()) {
        _file[row].removeAt(i);
    }
}

void Crdt::splitRowsBuf(int row, int column) {

    int i = column+1;

    while(i != _textBuffer[row].length()) {
        _textBuffer[row].removeAt(i);
    }
}

void Crdt::insertText(QChar val, Format format, QPair<int,int> rowCh) {

    int row = rowCh.first;
    int ch = rowCh.second;
    QPair<QString,Format> c(val,format);

    if(row == _textBuffer.length()) {
        _textBuffer.append(QList<QPair<QString,Format>>());
    }

    if(val == '\n' || val == '\r') {
        QList<QPair<QString,Format>> rowAfter = firstRowToEndLineBuf(rowCh);
        if(rowAfter.length() == 0) {
            _textBuffer[row].insert(ch, c);
            _textBuffer.insert(row+1, QList<QPair<QString,Format>>());
        }
        else {
            _textBuffer[row].insert(ch,c);
            splitRowsBuf(row,ch);
            _textBuffer.insert(row+1, rowAfter);
        }
    }
    else {
        _textBuffer[row].insert(ch,c);
    }
}

Char Crdt::generateChar(QChar val, QPair<int, int> rowCh, Format format) {

    QList<Identifier> posBefore = findPosBefore(rowCh);
    QList<Identifier> posAfter = findPosAfter(rowCh);
    QList<Identifier> newPos = generatePosBetween(posBefore, posAfter, QList<Identifier>());

    return Char(val, 0, _siteID, newPos, format);
}

QList<Identifier> Crdt::findPosBefore(QPair<int, int> rowCh) {

    int row = rowCh.first;
    int ch = rowCh.second;

    if(ch == 0 && row == 0) {
        return QList<Identifier>();
    }
    else if (ch == 0 && row != 0) { //first char of the row, but not the first one
        row -= 1;
        ch = _file[row].length();
    }

    return _file[row][ch-1]._position;
}

QList<Identifier> Crdt::findPosAfter(QPair<int, int> rowCh) {

    int row = rowCh.first;
    int ch = rowCh.second;
    int numRows = _file.length();//in theory this is the number of rows, debug needed

    if(numRows == 0) {
        return QList<Identifier>();
    }
    int numChars = _file.at(row).length();

    if((row == numRows-1) && (ch == numChars)) {
        return QList<Identifier>();
    }
    else if ((row < numRows-1) && (ch == numChars)) {
        row += 1;
        ch = 0;
    }
    else if ((row > numRows -1 && ch == 0)) {
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

bool Crdt::cornerCaseHandler(Char c){

    bool cornerCase = false;
    if(_file.isEmpty()) {
        cornerCase = true;
    }
    else if(_file.first().isEmpty()){
        cornerCase = true;
    }
    else if(c.compareTo(_file[0][0]) <= 0){
        cornerCase = true;
    }
    else
         cornerCase = false;

    return cornerCase;
}

QPair<int, int> Crdt::findInsertPosition(Char c) {

    if(cornerCaseHandler(c))
        return QPair<int, int>(0,0);

    int minRow = 0;
    int totalRows = _file.length();
    int maxRow = totalRows - 1;
    QList<Char> lastRow = _file[maxRow];
    int midRow;
    QList<Char> currentRow, minCurrentRow, maxCurrentRow;
    Char minLastChar, maxLastChar;
    int charIndex;

    Char lastC;

    if(lastRow.length()-1 < 0) {
        QList<Char> lastRow2 = _file[maxRow-1];
        lastC = lastRow2[lastRow2.length()-1];
    } else {
        lastC = lastRow[lastRow.length() - 1];
    }

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
    maxCurrentRow = _file[maxRow];
    maxLastChar = maxCurrentRow[maxCurrentRow.length()-1];

    if(c.compareTo(minLastChar) <= 0) {
        charIndex = findInsertIndexInLine(c, minCurrentRow);
        return QPair<int,int>(minRow,charIndex);
    } else {
        charIndex = findInsertIndexInLine(c, maxCurrentRow);
        return QPair<int,int>(maxRow,charIndex);
    }
}

QPair<int, int> Crdt::findEndPosition(Char lastC, QList<Char> lastRow, int totalLines) {
    if(lastC._value == '\r' || lastC._value == '\n') {
        return QPair<int,int>(totalLines-1,0);
    } else {
        return QPair<int,int>(totalLines-1, lastRow.length()); //-1?
    }
}

int Crdt::findInsertIndexInLine(Char c, QList<Char> row) {

    int left = 0;
    int right = row.length() - 1;
    int mid, compareNum;

    if (row.length() == 0 || c.compareTo(row[left]) < 0) {
      return left;
    } else if (c.compareTo(row[right]) > 0) {
      return row.length();
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

QString Crdt::getFileName(){
    return _fileName;
}

QList<QList<QPair<QString, Format>>> Crdt::getTextBuffer(){
    return _textBuffer;
}

QUuid Crdt::getSiteID() {
    return _siteID;
}


QPair<int,int> Crdt::findIndexInLine(Char c, QList<Char> row, int rowIndex){

    int left = 0;
    int right = row.length()- 1;
    int mid, compareNum;

    if (row.length() == 0 || c.compareTo(row[left]) < 0) {
      return QPair<int,int>(rowIndex,left);
    }

    while (left + 1 < right) {
      mid = qFloor(left + (right - left) / 2);
      compareNum = c.compareTo(row[mid]);

      if (compareNum == 0) {
        return QPair<int,int>(rowIndex,mid);
      }
      else if (compareNum > 0) {
        left = mid;
      }
      else {
        right = mid;
      }
    }

    if (c.compareTo(row[left]) == 0) {
      return QPair<int,int>(rowIndex,left);
    }
    else if (c.compareTo(row[right]) == 0) {
      return QPair<int,int>(rowIndex,right);
    }
    else {
        return QPair<int,int>();
    }
}

QPair<int,int> Crdt::findPosition(Char c) {

    int minRow = 0;
    int totalRows = _file.length();
    int maxRow = totalRows - 1;
    QList<Char> lastRow = _file[maxRow];
    Char lastCh, minLastCh, maxLastCh;
    int midRow;
    QList<Char> currRow, minCurrRow, maxCurrRow;
    QPair<int,int> position;

    if(cornerCaseHandler(c))
        return QPair<int, int>(0,0);

    if(lastRow.length()-1 < 0) {
        QList<Char> lastRow2 = _file[maxRow-1];
        lastCh = lastRow2[lastRow2.length()-1];
    } else {
        lastCh = lastRow[lastRow.length() - 1];
    }

    if(c.compareTo(lastCh) > 0) {
        return QPair<int,int>();
    }

    while(minRow + 1 < maxRow) {
        midRow = qFloor(minRow+(maxRow-minRow)/2);
        currRow = _file[midRow];
        lastCh = currRow[currRow.length()-1];

        if(c.compareTo(lastCh) == 0) {
            return QPair<int,int>(midRow, currRow.length()-1);
        } else if (c.compareTo(lastCh) < 0) {
            maxRow = midRow;
        } else {
            minRow = midRow;
        }
    }

    //Check between min and max row
    minCurrRow = _file[minRow];
    minLastCh = minCurrRow[minCurrRow.length()-1];
    maxCurrRow = _file[maxRow];
    if(maxCurrRow.length()==0) {
         QList<Char> beforeMaxRow = _file[maxRow-1];
         maxLastCh = beforeMaxRow[beforeMaxRow.length()-1];
    } else {
         maxLastCh = maxCurrRow[maxCurrRow.length()-1];
    }


    if (c.compareTo(minLastCh) <= 0) {
        position = findIndexInLine(c, minCurrRow, minRow);
    } else {
        position = findIndexInLine(c, maxCurrRow, maxRow);
    }

    return position;
}


QPair<int,int> Crdt::handleRemoteDelete(const QJsonObject &qjo, bool client) {

    Char c = getChar(qjo["content"].toObject());
    QPair<int,int> index = findPosition(c);
    _file[index.first].removeAt(index.second);

    if(client)
        _textBuffer[index.first].removeAt(index.second);

    if((c._value == '\r' || c._value == '\n') && index.first+1 != _file.length()) {
        mergeRows(index.first, client);
    }

    return index;
}

QPair<int,int> Crdt::handleRemoteInsert(const QJsonObject &qjo, bool client) {

    Char c = getChar(qjo["content"].toObject());
    QPair<int,int> position = findInsertPosition(c);
    insertChar(c, position);
    if(client)
        insertText(c._value,c._format,position);

    return position;
}


QPair<int,int> Crdt::handleRemoteFormat(const QJsonObject &qjo, bool client) {

    Char c = getChar(qjo["content"].toObject());
    QPair<int,int> index = findPosition(c);
    _file[index.first].replace(index.second, c);
    if(client)
        _textBuffer[index.first].replace(index.second, QPair<QString,Format>(c._value,c._format));

    return index;
}


void Crdt::removeChar(Char c, QPair<int,int> index) {

    _file[index.first].removeAt(index.second);

    if((c._value == '\r' || c._value == '\n') && index.first+1 != _file.length()) {
            mergeRows(index.first, false);
    }
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

QJsonObject Crdt::setChar(Char c) {
    QJsonObject jsonChar;
    jsonChar["value"] = QJsonValue(c._value);
    jsonChar["format"] = QJsonValue(c._format);
    jsonChar["siteID"] = QJsonValue(c._siteID.toString());
    jsonChar["counter"] = QJsonValue(c._counter);
    jsonChar["row"] = 0;
    jsonChar["column"] = 0;

    QJsonArray positions;
    foreach(const Identifier id, c._position) {
        QJsonObject pos;
        pos["digit"] = QJsonValue(id._digit);
        pos["siteID"] = QJsonValue(id._siteID.toString());
        positions.append(pos);
    }

    jsonChar["position"] = positions;
    return jsonChar;
}

QJsonObject Crdt::crdtToJson() {
    QJsonObject jsonFile;
    QJsonArray content;
    foreach(QList<Char> row, _file) {
        foreach(Char c, row) {
            QJsonObject jsonChar = setChar(c);
            content.append(jsonChar);
        }
    }
    jsonFile.insert("content", content);
    return jsonFile;
}

Format Crdt::getCurrentFormat(QPair<int,int> position) {
    if(position.second < 0 && position.first != 0) { //devo guardare il char alla riga precedente
        return _file[position.first-1].at(_file[position.first-1].length()-1)._format;
    }
    return _file[position.first].at(position.second)._format;
}

int Crdt::calcIndex(QPair<int, int> rowCh) {
    int index = 0;
    int row;
    for(row = 0; row < rowCh.first; row++)
        index += _file.at(row).length();
    index += rowCh.second;
    return index; //da modificare in -1 quando faccio la delete probably
}

bool Crdt::calcBeforePosition(QPair<int,int> start, QPair<int,int> & startBefore) {
    if(start.second - 1 == -1) {
        if(start.first - 1 < 0) {
            return false; // it's the first row
        } else {
            startBefore.first = start.first - 1;
            startBefore.second = _file[startBefore.first].length()-1;
        }
    }
    else {
        startBefore.first = start.first;
        startBefore.second = start.second - 1;
    }

    return true;
}

bool Crdt::calcAfterPosition(QPair<int,int> end, QPair<int,int> & endAfter) {

    if(_file[end.first].length() == 0) {
            return false;
        }

    if(end.second+1 > _file[end.first].length()) { //occhio alle righe insesistneti
        if(end.first + 1 == _file.length()) {
            return false;
        } else {
            endAfter.first = end.first + 1;
            endAfter.second = 0;
        }
    } else {
        endAfter.first = end.first;
        endAfter.second = end.second;
    }

    return true;
}

QList<QPair<QString,Format>> Crdt::takeMultipleBufRows(QPair<int,int> startPos, QPair<int,int> endPos) {
    QList<QPair<QString,Format>> chars;
    chars = firstRowToEndLineBuf(startPos);
    int row;
    for(row = startPos.first + 1; row < endPos.first; row++) {
        chars.append(_textBuffer[row]);
    }

    if(!_file[endPos.first].isEmpty()) {
        chars.append(lastRowToEndPosBuf(endPos)); //take the chars of the last selected row til endPos.ch
    }
    return chars;
}

QList<QPair<QString,Format>> Crdt::takeSingleBufRow(QPair<int,int> startPos, QPair<int,int> endPos) {
    return _textBuffer[startPos.first].mid(startPos.second,endPos.second);
}

QList<QPair<QString,Format>> Crdt::firstRowToEndLineBuf(QPair<int, int> startPos){
    // TODO: capire se va bene così oppure cambiarla concettualmente.
    QList<QPair<QString,Format>> buff = _textBuffer[startPos.first];
    QList<QPair<QString,Format>> res(buff.mid(startPos.second));
    return res;
}

QList<QPair<QString,Format>> Crdt::lastRowToEndPosBuf(QPair<int, int> endPos){
    QList<QPair<QString,Format>> buff = _textBuffer[endPos.first];
    QList<QPair<QString,Format>> res(buff.mid(0, endPos.second));
    return res;
}
