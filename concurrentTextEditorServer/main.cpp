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
}
