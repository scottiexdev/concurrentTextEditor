#ifndef WORKERCLIENT_H
#define WORKERCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>

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

private slots:
    void onReadyRead();

signals:
    void myLoggedIn();
    void mySignupOk();
    void showFiles(QStringList list);
    void showFileLine(QString buf);

private:
    QTcpSocket* _clientSocket;
    bool _loggedIn;
    QString _loggedUser;
    const QString DEFAULT_USER  = "unknownUsername";
    enum messageType  { login, filesRequest, fileRequest, invalid, signup };

    //Methods
    void jsonReceived(const QJsonObject &qjo);
    WorkerClient::messageType getMessageType(const QJsonObject &docObj);
    void loginHandler(const QJsonObject& jsonObj);
    void signupHandler(const QJsonObject& jsonObj);
    void showallFilesHandler(const QJsonObject& qjo);
    void showFileHandler(const QJsonObject& qjo);

};

#endif // WORKERCLIENT_H
