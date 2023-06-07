#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H
#include <QByteArray>
#include <QDebug>
#include <QMap>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>
#include <QString>

#include "parsing.h"

class MyTcpServer : public QObject {
    Q_OBJECT
public:
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();

    void handle_start_message(QTcpSocket *socket, QString login, QString roomname);
    void handle_break_message(QTcpSocket *socket);
    void handle_stats_message(QTcpSocket *socket);
    void handle_rooms_message(QTcpSocket *socket);
    void handle_newroom_message(QTcpSocket *socket, const QString &message);

public slots:
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerRead();

private:
    QTcpServer *mTcpServer;
    QMap<int, QTcpSocket *> mTcpSocket;
    int server_status;
    QMap<QString, QList<QString>> mQueues;
    Parsing *mParser;
};

#endif	// MYTCPSERVER_H
