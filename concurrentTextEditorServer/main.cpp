#include <QDebug>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <iostream>

//My includes
#include "server.h"
#include "serverhelpers.h"

int main(int argc, char *argv[])
{
   // if(!ServerHelper::InputCheck(argc, argv))
   //     return 0;

    std::cout<<"Starting server..."<<std::endl;

    //Create server
    Server server(nullptr, "Concurrent Server");

    //Connect to Database

    if(!server.ConnectToDatabase("QSQLITE", "path"))
        return 0;

    if(!(server.listen(QHostAddress("127.0.0.1"), 0)))
        std::cout<<"Unable to start server, connection refused"<<std::endl;
    else
        std::cout<<"Server listening on localhost, port 0,"<<std::endl;

    QTcpSocket baseSocket(nullptr);


    return 0;
}
