#pragma once
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include "data/packetbuffer.cpp"
#include <netinet/tcp.h>
#include "exception/net_exception.cpp"

std::string pingServer(std::string host, int port)
{
    struct sockaddr_in server_addr = {};

    int client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0)
    {
        throw net_exception("failed to create socket");
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_aton(host.c_str(), &server_addr.sin_addr);

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    if (connect(client, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        throw net_exception("failed to connect");
    }

    // Handshake packet
    // Send packet length

    PacketBuffer packetBuf;
    packetBuf.writeVarInt(0)->writeVarInt(340)->writeString("localhost")->writeUShort(25565)->writeVarInt(1);

    PacketBuffer lengthBuffer;
    lengthBuffer.writeVarInt(packetBuf.getSize());

    // write size to temp buffer
    std::vector<unsigned char> tempBuffer;
    std::vector<unsigned char> lengthData = lengthBuffer.toBytes();
    std::vector<unsigned char> actualData = packetBuf.toBytes();
    tempBuffer.insert(tempBuffer.end(), lengthData.begin(), lengthData.end());

    // write actula data to temp buffer
    tempBuffer.insert(tempBuffer.end(), actualData.begin(), actualData.end());

    PacketBuffer emptyPacket;
    emptyPacket.writeVarInt(1);
    emptyPacket.writeVarInt(0);

    std::vector<unsigned char> emptyBuffer = emptyPacket.toBytes();

    unsigned char *tempData = &tempBuffer[0];
    send(client, tempData, tempBuffer.size(), 0);

    unsigned char *emptyData = &emptyBuffer[0];
    send(client, emptyData, emptyBuffer.size(), 0);

    PacketBuffer clientBuf(client);

    // read length
    clientBuf.readVarInt();
    // read packet id
    clientBuf.readVarInt();
    std::string payload = clientBuf.readString();

    return payload;
}