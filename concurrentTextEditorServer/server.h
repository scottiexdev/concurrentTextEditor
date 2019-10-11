#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtNetwork/QTcpServer>
#include <iostream>
#include <string>
#include <QtSql>
#include <QFileInfo>
#include <QString>


class Server : public QTcpServer
{
    Q_OBJECT

public:
    //Constructors
    Server(QObject *parent) : QTcpServer(parent) {}
    Server(QObject *parent, std::string serverName) : QTcpServer(parent), _serverName(serverName) {}

    //Methods
    std::string GetName(void);
    bool ConnectToDatabase(QString database, QString connectionString);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:    
    std::string _serverName;
    QSqlDatabase db;
};

#endif // SERVER_H
