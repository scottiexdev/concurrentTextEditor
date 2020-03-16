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

    void getFileList(QString access);
    void requestFile(QString fileName, QUuid siteID, bool isPublic);
    void newFileRequest(const QJsonObject& qjo);
    void saveLinkToServer(const QJsonObject& qjo);

    void requestUserList(QString fileName);
    void userJoined(QString fileName, QString user);
    void userLeft(QString fileName, QString user);

private slots:
    void onReadyRead();

public slots:
    void broadcastEditWorker(QString fileName, Char c, EditType editType, int index, bool isPublic);

signals:
    void myLoggedIn();
    void mySignupOk();
    void genericError(QString str="Error");
    void showFiles(QStringList list, QStringList list2, QStringList list3, bool isPublic);
    void handleFile(QJsonDocument buf);
    void showUser(QString user);
    void deleteUser(QString user);
    void handleRemoteEdit(const QJsonObject& qjo);
    void ifFileOpenOk(const QJsonObject& qjo);


private:
    QTcpSocket* _clientSocket;
    bool _loggedIn;
    QString _loggedUser;
    const QString DEFAULT_USER  = "unknownUsername";

    //Methods
    void jsonReceived(const QJsonObject &qjo);
    void loginHandler(const QJsonObject& jsonObj);
    void signupHandler(const QJsonObject& jsonObj);
    void showallFilesHandler(const QJsonObject& qjo);
    void showUserListHandler(const QJsonObject& qjo);
    void newFileError();
    void sendJson(const QJsonObject &doc);

};

#endif // WORKERCLIENT_H
