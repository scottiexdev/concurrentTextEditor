#ifndef WORKERCLIENT_H
#define WORKERCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDir>
#include <QFileInfo>


class WorkerClient : public QObject
{
    Q_OBJECT

public:
    WorkerClient(QObject *parent= nullptr);
    void connectToServer(const QHostAddress& address, quint16 port);
<<<<<<< Updated upstream
    void SendLoginCred(QJsonObject qj);
    //bool receiveLoginResult();
=======
    void sendLoginCred(QJsonObject qj);
    void setUser(QString loggedUser);
    QString getUser();
    void getFileList();
>>>>>>> Stashed changes

private slots:
    void onReadyRead();

signals:
<<<<<<< Updated upstream
    void myLoggedIn();
=======
    void myLoggedIn(QString loggedUser);
    void mySignupOk(QString signedUser);
>>>>>>> Stashed changes

private:
    void jsonReceived(const QJsonObject &qjo);
    QTcpSocket* _clientSocket;
    bool _loggedIn;
<<<<<<< Updated upstream
=======
    QString _loggedUser;
    const QString DEFAULT_USER  = "unknownUsername";
    enum messageType  { signup, login, filesRequest, invalid };

    //Methods
    WorkerClient::messageType getMessageType(const QJsonObject &docObj);
    void jsonReceived(QJsonObject qjo);
    void loginHandler(QJsonObject& jsonObj);
    void signupHandler(QJsonObject& jsonObj);
    void showallFilesHandler(QJsonObject& qjo);
>>>>>>> Stashed changes

};

#endif // WORKERCLIENT_H
