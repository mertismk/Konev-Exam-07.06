#ifndef PARSING_H
#define PARSING_H

#include <QByteArray>
#include <QDebug>
#include <QMap>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>

class MyTcpServer;

class Parsing : public QObject {
    Q_OBJECT
public:
    explicit Parsing(MyTcpServer *server, QObject *parent = nullptr);
    void fparsing(QByteArray message, QTcpSocket *socket);
private:
    MyTcpServer *mServer;
};

#endif	// PARSING_H
