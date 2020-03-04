#ifndef WORKERCLIENT_H
#define WORKERCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QMessageBox>

#include "char.h"
#include "Enums.h"

class WorkerClient : public QObject
{
    Q_OBJECT

public:
    WorkerClient(QObject *parent= nullptr);

    void connectToServer(const QHostAddress& address, quint16 port);
    void sendLoginCred(QJsonObject qj);
    //bool receiveLoginResult();

    void setUser(QString loggedUser);
    QString getUser();

    void getFileList();
    void requestFile(QString fileName);
    void newFileRequest(const QJsonObject& qjo);

    void requestUserList(QString fileName);
    void userJoined(QString fileName, QString user);
    void userLeft(QString fileName, QString user);


private slots:
    void onReadyRead();

public slots:
    void broadcastEditWorker(QString fileName, Char c, EditType editType);

signals:
    void myLoggedIn();
    void mySignupOk();
    void genericError(QString str="Error");
    void showFiles(QStringList list, QStringList list2, QStringList list3);
    void handleFile(QJsonDocument buf);
    void showUser(QString user);
    void deleteUser(QString user);


private:
    QTcpSocket* _clientSocket;
    bool _loggedIn;
    QString _loggedUser;
    const QString DEFAULT_USER  = "unknownUsername";
    enum messageType  { login, filesRequest, invalid, signup, newFile, userListRequest };

    //Methods
    void jsonReceived(const QJsonObject &qjo);
    WorkerClient::messageType getMessageType(const QJsonObject &docObj);
    void loginHandler(const QJsonObject& jsonObj);
    void signupHandler(const QJsonObject& jsonObj);
    void showallFilesHandler(const QJsonObject& qjo);
    void showUserListHandler(const QJsonObject& qjo);
    void newFileError();
    void sendJson(const QJsonObject &doc);

};

#endif // WORKERCLIENT_H
