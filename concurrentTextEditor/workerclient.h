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
#include <QStandardPaths>
#include <QBuffer>
#include <QIcon>

#include "char.h"
#include "Enums.h"

class WorkerClient : public QObject
{
    Q_OBJECT

public:
    WorkerClient(QObject *parent= nullptr);

    bool connectToServer(const QHostAddress& address, quint16 port);
    void disconnectFromServer();
    void closeConnection();
    void sendLoginCred(QJsonObject qj);
    //bool receiveLoginResult();

    void setUser(QString loggedUser);
    void setIcon(QPixmap icon);
    QString getUser();
    QPixmap getUserIcon();
    void getCurrentIconFromServer();
    QByteArray getLatinStringFromImg(QString path);
    void getFileList(QString access);
    void requestFile(QString fileName, QUuid siteID, bool isPublic);
    void deleteFile(QString fileName, bool isPublic);
    void newFileRequest(const QJsonObject& qjo);
    void saveLinkToServer(const QJsonObject& qjo);
    void getSharedFile(QString link);
    void requestUserList(QString fileName);
    void userJoined(QString fileName, QString user);
    void userLeft(QString fileName, QString user);
    void changeProPic(QJsonObject &qj);
    void newUsername(QJsonObject &qj);
    void saveIcon(QJsonObject &qj);
    void setNewPassowrd(QString pwd);
    void setNewEmail(QString email);
    void getEditorUIIcons();
    QIcon getIcon(UiEditor tag);

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
    void fileDeleted();
    void disconnectClient();
    void newUsernameOk();
    void newUsernameNok();
    void iconSent(QPixmap icon);

private:
    QTcpSocket* _clientSocket;
    bool _loggedIn;
    QString _loggedUser;
    const QString DEFAULT_USER  = "unknownUsername";
    QPixmap _userIcon;

    QPixmap bold, bold_s, italics, italics_s, underlined, underlined_s, cut, copy, paste, pdf;

    //Methods
    QPixmap getPixmapFromJson(const QJsonValue &jv);
    void jsonReceived(const QJsonObject &qjo);
    void loginHandler(const QJsonObject& jsonObj);
    void signupHandler(const QJsonObject& jsonObj);
    void showallFilesHandler(const QJsonObject& qjo);
    void showUserListHandler(const QJsonObject& qjo);
    void currentIconHandler(const QJsonObject& qjo);
    void newFileError();
    void sendJson(const QJsonObject &doc);
    void newUsernameHandler(const QJsonObject &doc);
    void editorIconsHandler(const QJsonObject &doc);
//    void newEmailResponse(const QJsonObject &doc);
//    void newPasswordResponse(const QJsonObject &doc);

};

#endif // WORKERCLIENT_H
