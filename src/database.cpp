#include "database.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>

bool Database::connect()
{
    // Read db.txt file
    std::string connLine;
    try
    {
        std::ifstream file("db.txt");
        if (!file.good())
        {
            std::cout << "Failed to open db.txt" << std::endl;
            return false;
        }

        if (!std::getline(file, connLine)) {
            std::cout << "Failed to read the database URL from db.txt!" << std::endl;
            return false;
        }
    }
    catch (std::exception &ex)
    {
        std::cout << "Error occurred while reading db.txt: " << ex.what() << std::endl;
        return false;
    }

    try
    {
        pqxx::connection *conn = new pqxx::connection{connLine};

        this->conn = conn;
        return true;
    }
    catch (std::exception &ex)
    {
        std::cout << "Database connection failed:\n"
                  << ex.what() << std::endl;
        return false;
    }
}

void Database::setupDatabase()
{
    if (conn == NULL)
    {
        throw std::runtime_error("not connected");
    }

    pqxx::work tx{*conn};
    tx.exec(R"(CREATE TABLE IF NOT EXISTS server (
                    id SERIAL PRIMARY KEY,
                    host TEXT NOT NULL,
                    port INT NOT NULL,
                    motd TEXT NOT NULL,
                    protocolNumber INT NOT NULL,
                    versionName TEXT NOT NULL,
                    modType TEXT,
                    createdAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    latency INT,
                    maxPlayers INT NOT NULL,
                    playerCount INT NOT NULL
                );

                CREATE TABLE IF NOT EXISTS mod(
                    id SERIAL PRIMARY KEY,
                    mod_id TEXT NOT NULL,
                    version TEXT NOT NULL,
                    createdAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    server_id INT NOT NULL,
                    FOREIGN KEY (server_id) REFERENCES server (id)
                );

                CREATE TABLE IF NOT EXISTS player (
                    id SERIAL PRIMARY KEY,
                    uuid VARCHAR(36) NOT NULL,
                    username TEXT NOT NULL,
                    createdAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    server_id INT NOT NULL,
                    FOREIGN KEY (server_id) REFERENCES server (id)
                );
            )");

    tx.commit();
    // tx.exec("CREATE TABLE IF NOT EXISTS mod (id INT NOT NULL PRIMARY KEY AUTO_INCREMENT, mod_id VARCHAR(1000) NOT NULL)");
}

void Database::insertServerData(ServerData server)
{
    if (conn == NULL)
    {
        throw std::runtime_error("not connected");
    }

    std::string *modType = NULL; // = server.modList.has_value() ? &modTypeStr : NULL;
    std::string modTypeStr;
    if (server.modList.has_value())
    {
        modTypeStr = server.modList.value().type;
        modType = &modTypeStr;
    }

    // std::string modTypeStr = server.modList.value().type;

    mutex.lock();
    pqxx::work tx{*conn};
    pqxx::result serverRow = tx.exec_params("INSERT INTO server (host,port,motd,protocolNumber,versionName,modType,maxPlayers,playerCount) VALUES ($1, $2, $3, $4, $5, $6, $7, $8) RETURNING id;",
                                            server.host, server.port, server.descriptionJson, server.protocolNumber, server.versionName, modType, server.maxPlayers, server.playerCount);

    if (serverRow.empty())
    {
        throw std::runtime_error("failed to insert");
    }

    int serverId = serverRow[0][0].as<int>();

    tx.commit();
    mutex.unlock();

    if (server.modList.has_value())
    {
        ModList modList = server.modList.value();
        for (Mod mod : modList.mods)
        {
            mutex.lock();
            pqxx::work tx{*conn};
            tx.exec_params("INSERT INTO mod (mod_id,version,server_id) VALUES ($1, $2, $3);", mod.modId, mod.version, serverId);
            tx.commit();
            mutex.unlock();
        }
    }

    for (PlayerData plr : server.players)
    {
        mutex.lock();
        pqxx::work tx{*conn};
        tx.exec_params("INSERT INTO players (uuid,username,server_id) VALUES ($1, $2, $3);", plr.id, plr.name, serverId);
        tx.commit();
        mutex.unlock();
    }
}
