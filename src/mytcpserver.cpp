#include "mytcpserver.h"

MyTcpServer::~MyTcpServer() {
    // mTcpSocket->close();
    mTcpServer->close();
    server_status = 0;
    SingletonDB::instance().close();
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

    if (!SingletonDB::instance().open()) {
        qDebug() << "can't open db";
    }
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

    if (string.startsWith("start")) {
        handle_start_message(cur_mTcpSocket, string);
    } else if (string.startsWith("break")) {
        handle_break_message(cur_mTcpSocket);
    } else if (string.startsWith("stats")) {
        handle_stats_message(cur_mTcpSocket);
    } else if (string.startsWith("rooms")) {
        handle_rooms_message(cur_mTcpSocket);
    } else if (string.startsWith("newroom")) {
        handle_newroom_message(cur_mTcpSocket, string);
    } else {
        cur_mTcpSocket->write("Unknown command\r\n");
    }
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

void MyTcpServer::handle_start_message(QTcpSocket *socket, const QString &message) {
    QStringList parts = message.split("&");
    if (parts.size() < 3) {
        socket->write("Invalid message format\r\n");
        return;
    }
    QString login = parts[1];
    QString roomname = parts[2];

    if (mQueues.contains(roomname)) {
        QList<QString> clients = mQueues[roomname];
        if (clients.size() == 7) {
            QString response = "The queue is full. Clients in the queue:\r\n";
            for (QString client : clients) {
                response += "- " + client + "\r\n";
            }
            for (QTcpSocket *socket : mTcpSocket.values()) {
                socket->write(response.toUtf8());
                socket->disconnectFromHost();
            }
            mTcpServer->close();
            return;
        }
        clients.append(login);
        mQueues[roomname] = clients;
    } else {
        QList<QString> clients = { login };
        mQueues[roomname] = clients;
    }

    QString stats = "0,0,0";
    if (!SingletonDB::instance().addClient(login, roomname, stats)) {
        socket->write("Failed to store client info in the database\r\n");
        return;
    }
}

void MyTcpServer::handle_break_message(QTcpSocket *socket) {
    QString login;
    QString roomname;
    for (QString key : mQueues.keys()) {
        QList<QString> clients = mQueues[key];
        if (clients.contains(login)) {
            roomname = key;
            break;
        }
    }
    if (roomname.isEmpty()) {
        socket->write("You are not in queue\r\n");
        return;
    }
    QList<QString> clients = mQueues[roomname];
    clients.removeOne(login);
    mQueues[roomname] = clients;
}

void MyTcpServer::handle_stats_message(QTcpSocket *socket) {
    QString response = "List of clients in queues:\r\n";
    for (QString key : mQueues.keys()) {
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
    QString response = "List of all queues:\r\n";
    for (QString key : mQueues.keys()) {
        response += "- " + key + "\r\n";
    }
    socket->write(response.toUtf8());
}

void MyTcpServer::handle_newroom_message(QTcpSocket *socket, const QString &message) {
    QStringList parts = message.split("&");
    if (parts.size() < 2) {
        socket->write("Invalid message format\r\n");
        return;
    }
    QString roomname = parts[1];
    if (mQueues.contains(roomname)) {
        socket->write("Queue with this name already exists\r\n");
        return;
    }
    QList<QString> clients;
    mQueues[roomname] = clients;

    // Check if the queue already exists
    QSqlQuery query = SingletonDB::instance().execute("SELECT * FROM rooms WHERE name = ?");
    query.addBindValue(roomname);
    if (query.exec() && query.next()) {
        socket->write("Queue with this name already exists\r\n");
        return;
    }

    // Add the new queue to the database
    if (!SingletonDB::instance().addRoom(roomname)) {
        socket->write("Failed to create new queue in the database\r\n");
        return;
    } else {
        QList<QString> clients;
        mQueues[roomname] = clients;
    }
}
