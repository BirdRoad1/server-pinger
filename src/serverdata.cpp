#include "serverdata.hpp"
#include <iostream>
#include "exception/ping_parse_exception.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

ServerData parseServerData(std::string host, int port, std::string serverJson)
{
    json data = json::parse(serverJson);
    json version = data["version"];
    if (!version.is_object())
    {
        throw ping_parse_exception("missing version");
    }

    json versionName = version["name"];
    if (!versionName.is_string())
        throw ping_parse_exception("missing version name");
    json versionProtocol = version["protocol"];
    if (!versionProtocol.is_number_integer())
        throw ping_parse_exception("missing version protocol");

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
        throw ping_parse_exception("missing description");
    }

    json players = data["players"];
    if (!players.is_object())
        throw ping_parse_exception("missing players");

    json maxPlayers = players["max"];
    if (!maxPlayers.is_number_integer())
        throw ping_parse_exception("missing max players");

    json onlinePlayers = players["online"];
    if (!onlinePlayers.is_number_integer())
        throw ping_parse_exception("missing online players");

    std::optional<ModList> modList;

    json modJson = data["modinfo"];
    if (modJson.is_object())
    {
        json modType = modJson["type"];
        if (!modType.is_string())
            throw ping_parse_exception("invalid mod list type");

        std::vector<Mod> mods;

        json modListJson = modJson["modList"];
        if (!modListJson.is_array())
            throw ping_parse_exception("missing mod list");

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
    }

    ServerData serverData{
        host,
        port,
        onlinePlayers,
        maxPlayers,
        descriptionStr,
        versionProtocol,
        versionName,
        playersVec,
        faviconStr,
        modList};

    return serverData;
}