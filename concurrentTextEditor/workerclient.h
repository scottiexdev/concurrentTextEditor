#ifndef WORKERCLIENT_H
#define WORKERCLIENT_H

#include <QObject>
#include <QTcpSocket>

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

private slots:
    void onReadyRead();

signals:
    void myLoggedIn();
    void mySignupOk();

private:
    QTcpSocket* _clientSocket;
    bool _loggedIn;
    QString _loggedUser;
    const QString DEFAULT_USER  = "unknownUsername";
    enum messageType  { login, filesRequest, invalid, signup };

    //Methods
    void jsonReceived(const QJsonObject &qjo);
    WorkerClient::messageType getMessageType(const QJsonObject &docObj);
    void loginHandler(const QJsonObject& jsonObj);
    void signupHandler(const QJsonObject& jsonObj);
    void showallFilesHandler(const QJsonObject& qjo);

};

#endif // WORKERCLIENT_H
