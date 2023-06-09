#include "mytcpserver.h"

MyTcpServer::~MyTcpServer() {
    // mTcpSocket->close();
    mTcpServer->close();
    server_status = 0;
}

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent) {
    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection, this,
            &MyTcpServer::slotNewConnection);

    if (!mTcpServer->listen(QHostAddress::Any, 33333)) {
        qDebug() << "server is not started";
    } else {
        server_status = 1;
        qDebug() << "server is started";
    }

    mParser = new Parsing(this);
}

void MyTcpServer::slotNewConnection() {
    if (server_status == 1) {
        QTcpSocket *cur_mTcpSocket;
        cur_mTcpSocket = mTcpServer->nextPendingConnection();
        cur_mTcpSocket->write("Hello, World!!! I am echo server!\r\n");
        connect(cur_mTcpSocket, &QTcpSocket::readyRead, this,
                &MyTcpServer::slotServerRead);
        connect(cur_mTcpSocket, &QTcpSocket::disconnected, this,
                &MyTcpServer::slotClientDisconnected);
        cur_mTcpSocket->setObjectName(cur_mTcpSocket->peerAddress().toString());
        mTcpSocket.insert(cur_mTcpSocket->socketDescriptor(), cur_mTcpSocket);
        qDebug() << "new connection " << cur_mTcpSocket->socketDescriptor();
    }
}

void MyTcpServer::slotServerRead() {
    // QTcpSocket *cur_mTcpSocket = mTcpSocket[mTcpServer->socketDescriptor()];
    QTcpSocket *cur_mTcpSocket = (QTcpSocket *)sender();
    QByteArray array;
    QString string;
    while (cur_mTcpSocket->bytesAvailable() > 0) {
        array = cur_mTcpSocket->readAll();
        string.append(array);
    }
    string = string.left(string.length() - 2);
    qDebug() << string;

    mParser->fparsing(string.toUtf8(), cur_mTcpSocket);
}

void MyTcpServer::slotClientDisconnected() {
    QTcpSocket *cur_mTcpSocket = (QTcpSocket *)sender();
    cur_mTcpSocket->close();
    auto iter = mTcpSocket.begin();
    while (iter.value() != cur_mTcpSocket) iter++;
    mTcpSocket.erase(iter);
    qDebug() << "client disconnected. number of clients = " << mTcpSocket.size();

    /*QTcpSocket *cur_mTcpSocket = mTcpSocket[mTcpServer->socketDescriptor()];
  cur_mTcpSocket->close(); */
}

void MyTcpServer::handle_start_message(QTcpSocket *socket, QString login, QString roomname) {
    if (!mQueues.contains(roomname)) {
        socket->write("Queue does not exist\r\n");
        return;
    }
    QList<QString> clients = mQueues[roomname];
    if (clients.contains(login)) {
        socket->write("You are already in the queue\r\n");
        return;
    }
    mQueues[roomname].append(login);
    socket->write("You are in the queue\r\n");
}

void MyTcpServer::handle_break_message(QTcpSocket *socket) {
    QString login;
    QString roomname;
    for (QString key : mQueues.keys()) {
        QList<QString> clients = mQueues[key];
        int index = clients.indexOf(socket->objectName());
        if (index != -1) {
            login = clients[index];
            roomname = key;
            clients.removeAt(index);
            mQueues[key] = clients;
            break;
        }
    }
    if (!login.isEmpty()) {
        QString response = "You have been removed from queue " + roomname + "\r\n";
        socket->write(response.toUtf8());
    } else {
        socket->write("You are not in any queue\r\n");
    }
}

void MyTcpServer::handle_stats_message(QTcpSocket *socket) {
    QString response = "List of clients in queues:\r\n";
    for (auto it = mQueues.begin(); it != mQueues.end(); ++it) {
        QString key = it.key();
        response += "Queue " + key + ":\r\n";
        QList<QString> clients = mQueues[key];
        if (clients.isEmpty()) {
            response += "- No clients in queue\r\n";
        } else {
            for (QString client : clients) {
                response += "- " + client + "\r\n";
            }
        }
    }
    socket->write(response.toUtf8());
}

void MyTcpServer::handle_rooms_message(QTcpSocket *socket) {
    QString response = "List of rooms:\r\n";
    for (QString key : mQueues.keys()) {
        response += "- " + key + "\r\n";
    }
    socket->write(response.toUtf8());
}

void MyTcpServer::handle_newroom_message(QTcpSocket *socket, const QString &message) {
    QList<QString> parts = message.split('&');
    if (parts.size() < 2) {
        socket->write("Invalid message format\r\n");
        return;
    }
    QString roomname = parts[1];
    if (mQueues.contains(roomname)) {
        socket->write("Room already exists\r\n");
        return;
    }
    mQueues[roomname] = QList<QString>();
    socket->write("Room created\r\n");
}
