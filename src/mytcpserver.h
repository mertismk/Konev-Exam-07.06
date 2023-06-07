#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include "singletondb.h"

#include <QByteArray>
#include <QDebug>
#include <QMap>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>

class MyTcpServer : public QObject {
    Q_OBJECT
public:
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();

    void handle_start_message(QTcpSocket *socket, const QString &message);
    void handle_break_message(QTcpSocket *socket);
    void handle_stats_message(QTcpSocket *socket);
    void handle_rooms_message(QTcpSocket *socket);
    void handle_newroom_message(QTcpSocket *socket, const QString &message);
public slots:
    void slotNewConnection();
    void slotClientDisconnected();

    void slotServerRead();
    // void slotReadClient();
private:
    QTcpServer *mTcpServer;
    QMap<int, QTcpSocket *> mTcpSocket;
    int server_status;
    QMap<QString, QList<QString>> mQueues;
};

#endif  // MYTCPSERVER_H
