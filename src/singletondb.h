#ifndef SINGLETONDB_H
#define SINGLETONDB_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>

class SingletonDB
{
public:
    static SingletonDB& instance();

    bool open();
    void close();
    QSqlQuery execute(const QString& sql);

    bool addClient(const QString& login, const QString& roomname, const QString& stats);
    bool addRoom(const QString& name);

private:
    SingletonDB();
    SingletonDB(const SingletonDB&) = delete;
    SingletonDB& operator=(const SingletonDB&) = delete;

    QSqlDatabase mDatabase;
};

#endif // SINGLETONDB_H
