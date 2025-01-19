#include "serverdata.hpp"
#include "exception/ping_parse_exception.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

ServerData parseServerData(std::string host, int port, const std::string& serverJson)
{
    try
    {
        ServerData serverData{};
        serverData.host = host;
        serverData.port = port;

        json data = json::parse(serverJson);
        json version = data["version"];
        if (version.is_object())
        {
            json versionName = version["name"];
            if (versionName.is_string())
            {
                serverData.versionName = versionName;
            }

            json versionProtocol = version["protocol"];
            if (versionProtocol.is_number_integer())
            {
                serverData.protocolNumber = versionProtocol;
            }
        }

        json favicon = data["favicon"];
        if (favicon.is_string())
        {
            serverData.favicon = favicon;
        }

        json description = data["description"];
        if (description.is_object())
        {
            serverData.descriptionJson = description.dump();
        }
        else if (description.is_string())
        {
            serverData.descriptionJson = description;
        }

        json players = data["players"];
        if (players.is_object())
        {
            json maxPlayers = players["max"];
            if (maxPlayers.is_number_integer())
            {
                serverData.maxPlayers = maxPlayers;
            }

            json onlinePlayers = players["online"];
            if (onlinePlayers.is_number_integer())
            {
                serverData.playerCount = onlinePlayers;
            }

            json playersJson = players["sample"];
            if (playersJson.is_array())
            {
                for (json playerJson : playersJson)
                {
                    json id = playerJson["id"];
                    if (!id.is_string())
                    {
                        continue; // uuid is required
                    }

                    PlayerData plrData{};
                    plrData.id = id;

                    json name = playerJson["name"];
                    if (name.is_string())
                    {
                        plrData.name = name;
                    }

                    serverData.players.push_back(plrData);
                }
            }
        }

        json modJson = data["modinfo"];
        if (modJson.is_object())
        {
            ModList modList{};

            json modType = modJson["type"];
            if (modType.is_string())
            {
                modList.type = modType;

                json modListJson = modJson["modList"];
                if (modListJson.is_array())
                {
                    for (json modObj : modListJson)
                    {
                        json modId = modObj["modid"];
                        if (!modId.is_string())
                        {
                            continue; // mod id is required
                        }

                        Mod mod{};
                        mod.modId = modId;

                        json modVersion = modObj["version"];
                        if (modVersion.is_string())
                        {
                            mod.version = modVersion;
                        }

                        modList.mods.push_back(mod);
                    }
                }

                serverData.modList = modList;
            }
        }

        return serverData;
    }
    catch (std::exception &ex)
    {
        throw ping_parse_exception(ex.what());
    }
}