# concurrentTextEditor
Concurrent text editor project. 

Naming conventions
 - Types start with capitals: MyClass - MyTemplate - MyNamespace
 - functions and variables start with lower case: myMethod
 - constants are all capital: const int PI=3.14159265358979323;
 - private member variables start with  "_" e.g: private: int _x;

Practical coding notes:
 - Usare il passaggio delle variabile per reference e non i puntatori: se passiamo per reference non dobbiamo preoccuparci che il puntatore possa avere valore nullo: si scatena un'eccezione automaticamente
 
 Sending a file in JSON:
  - https://forum.qt.io/topic/56319/solved-pass-a-binary-file-in-json/3
  CLIENT:
m_jsonObject["data"] = QString(data.toBase64());

SERVER:
QByteArray ba;
ba.append(json["data"].toString());
QByteArray data = QByteArray::fromBase64(ba);

It works fine :)
