#pragma once
#include <string>
#include <optional>
#include <vector>
#include "nlohmann/json.hpp"
#include <iostream>
#include "exception/ping_parse_exception.cpp"

using json = nlohmann::json;

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

ServerData parseServerData(std::string host, int port, std::string serverJson)
{
    json data = json::parse(serverJson);
    json version = data["version"];
    if (!version.is_object())
    {
        throw PingParseException("missing version");
    }

    json versionName = version["name"];
    if (!versionName.is_string())
        throw PingParseException("missing version name");
    json versionProtocol = version["protocol"];
    if (!versionProtocol.is_number_integer())
        throw PingParseException("missing version protocol");

    json favicon = data["favicon"];
    std::optional<std::string> faviconStr = favicon.is_string() ? std::optional(favicon) : std::nullopt;

    std::string descriptionStr;
    json description = data["description"];
    if (description.is_object())
    {
        descriptionStr = description.dump();
    }
    else if (description.is_string())
    {
        descriptionStr = description;
    }
    else
    {
        throw PingParseException("missing description");
    }

    json players = data["players"];
    if (!players.is_object())
        throw PingParseException("missing players");

    json maxPlayers = players["max"];
    if (!maxPlayers.is_number_integer())
        throw PingParseException("missing max players");

    json onlinePlayers = players["online"];
    if (!onlinePlayers.is_number_integer())
        throw PingParseException("missing online players");

    std::optional<ModList> modList;

    json modJson = data["modinfo"];
    if (modJson.is_object())
    {
        json modType = modJson["type"];
        if (!modType.is_string())
            throw PingParseException("invalid mod list type");

        std::vector<Mod> mods;

        json modListJson = modJson["modList"];
        if (!modListJson.is_array())
            throw PingParseException("missing mod list");

        for (json modObj : modListJson)
        {
            json modId = modObj["modid"];
            json modVersion = modObj["version"];
            if (!modId.is_string() || !modVersion.is_string())
            {
                std::cout << "invalid mod id/version" << std::endl;
                continue;
            }

            Mod mod{
                modId,
                modVersion};

            mods.push_back(mod);
        }

        ModList newModList{modType, mods};

        modList = std::optional(newModList);
    }

    std::vector<PlayerData> playersVec;

    json playersJson = players["sample"];
    if (modJson.is_array())
    {
        for (json playerJson : modJson)
        {
            json id = playerJson["id"];
            json name = playerJson["name"];
            if (!id.is_string() || !name.is_string())
            {
                std::cout << "invalid mod id/version" << std::endl;
                continue;
            }

            PlayerData plr{
                id,
                name};

            playersVec.push_back(plr);
        }
        // json modType = modJson["type"];
        // if (!modType.is_string())
        //     throw PingParseException("invalid mod list type");

        // std::vector<Mod> mods;

        // json modListJson = modJson["modList"];
        // if (!modListJson.is_array())
        //     throw PingParseException("missing mod list");

        // for (json modObj : modListJson)
        // {
        //     json modId = modObj["modid"];
        //     json modVersion = modObj["version"];
        //     if (!modId.is_string() || !modVersion.is_string())
        //     {
        //         std::cout << "invalid mod id/version" << std::endl;
        //         continue;
        //     }

        //     Mod mod {
        //         modId,
        //         modVersion};

        //     mods.push_back(mod);
        // }

        // ModList newModList{modType, mods};

        // modList = std::optional(newModList);
    }

    ServerData serverData{
        host,
        port,
        onlinePlayers,
        maxPlayers,
        descriptionStr,
        versionProtocol,
        versionName, playersVec, faviconStr,
        modList};

    return serverData;
}