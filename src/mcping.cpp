#include "mcping.hpp"
#include "data/packetbuffer.hpp"
#include <sys/socket.h>
#include "exception/net_exception.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

/**
 * Send Server List Ping request to server and returns the response
 * If there is no response or something goes wrong, `net_exception` is thrown
 */
std::string pingServer(const std::string &host, const int port) {
    sockaddr_in server_addr = {};

    // create socket
    int client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0) {
        throw net_exception("failed to create socket");
    }

    // setup socket address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_aton(host.c_str(), &server_addr.sin_addr);

    // setup socket timeout
    timeval timeout{};
    timeout.tv_sec = 6;
    timeout.tv_usec = 0;

    setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    // connect to server
    if (connect(client, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
        close(client);
        throw net_exception("failed to connect");
    }

    // Handshake packet
    PacketBuffer packetBuf;
    packetBuf
            .writeVarInt(0) // packet id 0x0 (handshaking)
            ->writeVarInt(340) // protocol version (1.12.2)
            ->writeString("localhost") // server hostname
            ->writeUShort(25565) // server port
            ->writeVarInt(1); // next state (status)

    // holds packetBuf's length
    PacketBuffer lengthBuffer;
    lengthBuffer.writeVarInt(static_cast<int>(packetBuf.getSize()));

    // holds length + data
    std::vector<unsigned char> tempBuffer;
    // holds length bytes
    std::vector<unsigned char> lengthData = lengthBuffer.toBytes();
    // holds data bytes
    std::vector<unsigned char> actualData = packetBuf.toBytes();

    // Append length bytes to the temp buffer
    tempBuffer.insert(tempBuffer.end(), lengthData.begin(), lengthData.end());

    // Append data bytes to the temp buffer
    tempBuffer.insert(tempBuffer.end(), actualData.begin(), actualData.end());

    // Send handshake packet
    unsigned char *tempData = &tempBuffer[0];
    if (send(client, tempData, tempBuffer.size(), 0) != static_cast<long int>(tempBuffer.size())) {
        throw net_exception("failed to send()");
    }

    // Empty 0x0 handshake packet
    PacketBuffer emptyPacket;
    emptyPacket.writeVarInt(1); // packet length
    emptyPacket.writeVarInt(0); // packet id (handshaking)

    // Send packet
    std::vector<unsigned char> emptyBuffer = emptyPacket.toBytes();
    unsigned char *emptyData = &emptyBuffer[0];
    if (send(client, emptyData, emptyBuffer.size(), 0) != static_cast<long int>(emptyBuffer.size())) {
        throw net_exception("failed to send()");
    }

    // Read server list ping response packet
    PacketBuffer clientBuf(client);

    // read packet length
    clientBuf.readVarInt();

    // read packet id
    clientBuf.readVarInt();

    // read server list ping JSON response
    std::string json = clientBuf.readString();

    close(client);

    return json;
}
