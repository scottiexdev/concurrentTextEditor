#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QWidget>

namespace Ui {
class ServerWindow;
}

class Server;
class ServerWindow : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ServerWindow)

public:
    explicit ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private:
    Ui::ServerWindow *ui;
    Server* m_server;

private slots:
    void toggleStartServer();
    void logMessage(const QString &msg);
};

#endif // SERVERWINDOW_H
