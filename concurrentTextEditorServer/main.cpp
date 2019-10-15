#include <QDebug>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <iostream>

//My includes
#include "server.h"
#include "serverhelpers.h"
#include "serverwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ServerWindow serverWin;
    serverWin.show();
    return a.exec();
//   // if(!ServerHelper::InputCheck(argc, argv))
//   //     return 0;

//    std::cout<<"Starting server..."<<std::endl;

//    //Create server
//    Server server("Concurrent Server");

//    //Connect to Database
//    if(!server.ConnectToDatabase())
//        return 0;

//    server.queryDatabase("select * from users");

//    if(!(server.listen(QHostAddress("127.0.0.1"), 0)))
//        std::cout<<"Unable to start server, connection refused"<<std::endl;
//    else
//        std::cout<<"Server listening on localhost, port 0,"<<std::endl;


//    QTcpSocket baseSocket(nullptr);

//    while (1) {
//        server.waitForNewConnection();
//    }


//    return 0;
}
