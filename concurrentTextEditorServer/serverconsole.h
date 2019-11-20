#ifndef SERVERCONSOLE_H
#define SERVERCONSOLE_H

#include <QWidget>
#include <QPlainTextEdit>

class ServerConsole : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit ServerConsole(QWidget *parent = nullptr);
    void keyPressEvent(QKeyEvent *key);

signals:
    void executeCommand(QString cmd);

public slots:
};

#endif // SERVERCONSOLE_H
