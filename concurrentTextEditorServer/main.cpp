#include <QCoreApplication>
#include <iostream>
#include <QtCore/QCoreApplication>
#include <QDebug>
#include <QtCore/QCoreApplication>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

//My includes
#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::cout<<"Starting server..."<<std::endl;

    //Create server
    Server server(nullptr, "Concurrent Server");

    if(!server.listen(QHostAddress("127.0.0.1"), 0)){
        std::cout<<"Unable to start server, connection refused"<<std::endl;
    }
    else{
        std::cout<<"Server listening on localhost, port 0,"<<std::endl;
    }

    QTcpSocket baseSocket(nullptr);



    return a.exec();
}
