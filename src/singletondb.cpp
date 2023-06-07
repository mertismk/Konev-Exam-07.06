#include "SingletonDB.h"

#include <QDebug>

SingletonDB& SingletonDB::instance() {
    static SingletonDB db;
    return db;
}

SingletonDB::SingletonDB() : mDatabase(QSqlDatabase::addDatabase("QSQLITE")) {
    mDatabase.setDatabaseName("database.db");
    qDebug() << "i'm here\n";
    if (!mDatabase.open()) {
        qDebug() << "can't open db";
    } else {
        QSqlQuery query;
        bool ok;
        qDebug() << "i'm hereee\n";
        ok = query.exec("CREATE TABLE IF NOT EXISTS clients ("
                        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "login TEXT NOT NULL,"
                        "roomname TEXT NOT NULL,"
                        "stats TEXT NOT NULL)");
        if (!ok) {
            qDebug() << "can't create table in db";
        }

        ok = query.exec("CREATE TABLE IF NOT EXISTS rooms ("
                        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "name TEXT NOT NULL)");
        if (!ok) {
            qDebug() << "can't create table db";
        }
    }
}

//bool SingletonDB::open() {
//    qDebug() << "i'm here open\n";
//    if (!mDatabase.isOpen()) {
//        mDatabase = QSqlDatabase::addDatabase("QSQLITE");
//        mDatabase.setDatabaseName("database.db");
//        if (!mDatabase.open()) {
//            return false;
//        }
//    }
//    return true;
//}

void SingletonDB::close() {
    mDatabase.close();
}

QSqlQuery SingletonDB::execute(const QString& sql) {
    QSqlQuery query(QSqlDatabase::database("database.db"));
    query.prepare(sql);
    return query;
}

bool SingletonDB::addClient(const QString& login, const QString& roomname, const QString& stats)
{
    QSqlQuery query(mDatabase);
    query.prepare("INSERT INTO clients (login, roomname, stats) VALUES (?, ?, ?)");
    query.addBindValue(login);
    query.addBindValue(roomname);
    query.addBindValue(stats);
    if (!query.exec()) {
        qDebug() << "can't insert clients";
        return false;
    }
    return true;
}

bool SingletonDB::addRoom(const QString& name)
{
    QSqlQuery query(mDatabase);
    query.prepare("INSERT INTO rooms (name) VALUES (?)");
    query.addBindValue(name);
    if (!query.exec()) {
        qDebug() << "can't insert rooms";
        return false;
    }
    return true;
}
