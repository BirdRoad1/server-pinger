#pragma once
#include <string>
#include <vector>
#include <optional>

struct PlayerData
{
    std::string id;
    std::string name;
};

struct Mod
{
    std::string modId;
    std::string version;
};

struct ModList
{
    std::string type;
    std::vector<Mod> mods;
};

struct ServerData
{
    std::string host;
    int port;
    int playerCount;
    int maxPlayers;
    std::string descriptionJson;
    int protocolNumber;
    std::string versionName;
    std::vector<PlayerData> players;
    std::optional<std::string> favicon;
    std::optional<ModList> modList;
};

ServerData parseServerData(std::string host, int port, std::string serverJson);