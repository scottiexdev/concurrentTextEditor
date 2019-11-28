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
    void newFileRequest(const QJsonObject& qjo);

private slots:
    void onReadyRead();

signals:
    void myLoggedIn();
    void mySignupOk();
    void genericError(QString str="Error");
    void showFiles(QStringList list);
    void showFileLine(QString buf);

private:
    QTcpSocket* _clientSocket;
    bool _loggedIn;
    QString _loggedUser;
    const QString DEFAULT_USER  = "unknownUsername";
    enum messageType  { login, filesRequest, invalid, signup, newFile };

    //Methods
    void jsonReceived(const QJsonObject &qjo);
    WorkerClient::messageType getMessageType(const QJsonObject &docObj);
    void loginHandler(const QJsonObject& jsonObj);
    void signupHandler(const QJsonObject& jsonObj);
    void showallFilesHandler(const QJsonObject& qjo);
    void newFileError();

};

#endif // WORKERCLIENT_H
