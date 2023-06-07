#include "parsing.h"
#include "mytcpserver.h"

Parsing::Parsing(MyTcpServer *server, QObject *parent) : QObject(parent), mServer(server) { }

void Parsing::fparsing(QByteArray message, QTcpSocket *socket) {
    QList<QByteArray> parts = message.split('&');
    QTcpSocket *cur_mTcpSocket = (QTcpSocket *)sender();
    qDebug() << parts[0];
    if (parts[0] == "start") {
        mServer->handle_start_message(cur_mTcpSocket, parts[1], parts[2]);
        qDebug() << "start ok";
    } else if (parts[0] == "break") {
        mServer->handle_break_message(cur_mTcpSocket);
        qDebug() << "break ok";
    } else if (parts[0] == "stats") {
        mServer->handle_stats_message(cur_mTcpSocket);
        qDebug() << "stat ok";
    } else if (parts[0] == "rooms") {
        mServer->handle_rooms_message(cur_mTcpSocket);
        qDebug() << "rooms ok";
    } else if (parts[0] == "newroom") {
        mServer->handle_newroom_message(cur_mTcpSocket, message);
        qDebug() << "newroom ok";
    } else
        qDebug() << "unknown command";
}
