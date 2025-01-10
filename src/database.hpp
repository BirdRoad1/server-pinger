#pragma once
#include <pqxx/pqxx>
#include "serverdata.hpp"
#include <mutex>

class Database
{
private:
    std::mutex mutex;
    pqxx::connection *conn = NULL;

public:
    ~Database()
    {
        if (conn != NULL)
        {
            delete conn;
        }
    }

    bool connect();

    void setupDatabase();

    void insertServerData(ServerData server);
};