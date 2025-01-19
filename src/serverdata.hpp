#pragma once
#include <string>
#include <vector>
#include <optional>

struct PlayerData
{
    std::string id;
    std::optional<std::string> name;
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

    std::optional<int> playerCount;
    std::optional<int> maxPlayers;
    std::optional<std::string> descriptionJson;
    std::optional<int> protocolNumber;
    std::optional<std::string> versionName;
    std::vector<PlayerData> players;
    std::optional<std::string> favicon;
    std::optional<ModList> modList;
};

ServerData parseServerData(std::string host, int port, const std::string& serverJson);