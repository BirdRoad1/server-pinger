#pragma once
#include <pqxx/pqxx>
#include "serverdata.hpp"
#include <mutex>

class Database {
private:
    std::mutex mutex;
    pqxx::connection *conn = nullptr;

public:
    ~Database() {
        delete conn;
    }

    bool connect();

    void setupDatabase() const;

    void insertServerData(ServerData server);
};
