#include "crdt.h"


Crdt::Crdt()
{

     _siteID = createQuuid();

}

QUuid Crdt::createQuuid(){

    auto generator = QRandomGenerator();
    QVector<int> seeds;

    for(int i = 0; i < 11; i++){
        int seed = generator.bounded(10000);
        seeds.push_front(seed);
    }

    return QUuid(seeds[0],seeds[1],seeds[2],seeds[3],seeds[4],seeds[5],seeds[6],seeds[7],seeds[8],seeds[9],seeds[10]);
}

bool Crdt::parseCteFile(QJsonDocument unparsedFile){
    _textBuffer = parseFile(unparsedFile);
    return true;
}


QString Crdt::parseFile(QJsonDocument unparsedFile){
    //TODO: parser
    _fileName = unparsedFile["fileName"].toString();

}

QString Crdt::getFileName(){

    if(_fileName.isNull() || _fileName.isEmpty()){
        //throw exception
    }

    return _fileName;
}

QString Crdt::getTextBuffer(){

    if(_textBuffer.isNull() || _textBuffer.isEmpty()){
        //throw exception
    }
    return _textBuffer;
}
