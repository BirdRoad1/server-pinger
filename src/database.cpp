#include "database.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>

bool Database::connect() {
    // Read db.txt file
    std::string connLine;
    try {
        std::ifstream file("db.txt");
        if (!file.good()) {
            logger->error()->write("Failed to open db.txt")->end();
            return false;
        }

        if (!std::getline(file, connLine)) {
            logger->error()->write("Failed to read the database URL from db.txt!")->end();
            return false;
        }

        file.close();
    } catch (std::exception &ex) {
        logger->error()->write("Error occurred while reading db.txt: ")->write(ex.what())->end();
        return false;
    }

    try {
        auto *conn = new pqxx::connection{connLine};

        this->conn = conn;
        return true;
    } catch (std::exception &ex) {
        logger->error()->write("Database connection failed:\n")->write(ex.what())->end();
        return false;
    }
}

void Database::setupDatabase() const {
    if (conn == nullptr) {
        throw std::runtime_error("not connected");
    }

    pqxx::work tx{*conn};
    tx.exec(R"(CREATE TABLE IF NOT EXISTS server (
                    id SERIAL PRIMARY KEY,
                    host TEXT NOT NULL,
                    port INT NOT NULL,
                    motd TEXT,
                    protocolNumber INT,
                    versionName TEXT,
                    modType TEXT,
                    createdAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    latency INT,
                    maxPlayers INT,
                    playerCount INT
                );

                CREATE TABLE IF NOT EXISTS mod(
                    id SERIAL PRIMARY KEY,
                    mod_id TEXT NOT NULL,
                    version TEXT,
                    createdAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    server_id INT,
                    FOREIGN KEY (server_id) REFERENCES server (id)
                );

                CREATE TABLE IF NOT EXISTS player (
                    id SERIAL PRIMARY KEY,
                    uuid VARCHAR(36) NOT NULL,
                    username TEXT,
                    createdAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    server_id INT,
                    FOREIGN KEY (server_id) REFERENCES server (id)
                );
            )");

    tx.commit();
}

void Database::insertServerData(ServerData server) {
    if (conn == nullptr) {
        throw std::runtime_error("not connected");
    }

    std::optional<std::string> modTypeStr;
    if (server.modList.has_value()) {
        modTypeStr = server.modList.value().type;
    }

    std::unique_lock lock(mutex);
    pqxx::work tx{*conn};
    pqxx::result serverRow = tx.exec_params(
        "INSERT INTO server (host,port,motd,protocolNumber,versionName,modType,maxPlayers,playerCount) VALUES ($1, $2, $3, $4, $5, $6, $7, $8) RETURNING id;",
        server.host, server.port, server.descriptionJson, server.protocolNumber, server.versionName, modTypeStr,
        server.maxPlayers, server.playerCount);

    if (serverRow.empty()) {
        throw std::runtime_error("inserted row empty");
    }

    int serverId = serverRow[0][0].as<int>();

    tx.commit();
    lock.unlock();

    if (server.modList.has_value()) {
        ModList modList = server.modList.value();
        for (const Mod &mod: modList.mods) {
            std::lock_guard lock(mutex);
            pqxx::work tx{*conn};
            tx.exec_params("INSERT INTO mod (mod_id,version,server_id) VALUES ($1, $2, $3);", mod.modId, mod.version,
                           serverId);
            tx.commit();
        }
    }

    for (const PlayerData &plr: server.players) {
        std::lock_guard lock(mutex);
        pqxx::work tx{*conn};
        tx.exec_params("INSERT INTO player (uuid,username,server_id) VALUES ($1, $2, $3);", plr.id, plr.name, serverId);
        tx.commit();
    }
}
