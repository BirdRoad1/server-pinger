#include "packetbuffer.hpp"
#include <arpa/inet.h> // For htons()
#include <iostream>
#include <stdexcept>
#include "../exception/net_exception.hpp"

unsigned char PacketBuffer::readByte()
{
    if (fd < 0)
    {
        if (cursor >= vec.size())
        {
            throw net_exception("Read failed!");
        }

        unsigned char byte = vec.at(cursor);
        cursor++;

        return byte;
    }
    else
    {
        unsigned char bytes[1];

        int len = recv(fd, bytes, sizeof(bytes), 0);
        if (len != 1)
        {
            throw net_exception("Failed to read byte from socket");
        }

        return bytes[0];
    }
}

void PacketBuffer::writeByte(unsigned char byte)
{
    if (fd < 0)
    {
        vec.push_back(byte);
        cursor++;
    }
    else
    {
        unsigned char bytes[1] = {byte};
        int len = send(fd, bytes, sizeof(bytes), 0);
        if (len != 1)
        {
            throw net_exception("Failed to write byte to socket");
        }
    }
}

PacketBuffer *PacketBuffer::writeVarInt(int value)
{
    while (true)
    {
        if ((value & ~SEGMENT_BITS) == 0)
        {
            writeByte(static_cast<unsigned char>(value));

            return this;
        }

        writeByte(static_cast<unsigned char>((value & SEGMENT_BITS) | CONTINUE_BIT));

        value = (int)((unsigned int)value >> 7);
    }

    return this;
}

int PacketBuffer::readVarInt()
{
    int value = 0;
    int position = 0;
    unsigned char currentByte;

    while (true)
    {
        currentByte = readByte();

        value |= (currentByte & SEGMENT_BITS) << position;

        if ((currentByte & CONTINUE_BIT) == 0)
            break;

        position += 7;

        if (position >= 32)
            throw net_exception("VarInt is too big");
    }

    return value;
}

unsigned short PacketBuffer::readUShort()
{
    return (readByte() << 8) | readByte();
}

PacketBuffer *PacketBuffer::writeUShort(unsigned short value)
{
    uint16_t networkValue = htons(value);

    writeByte(networkValue & 0xFF);        // Low byte
    writeByte((networkValue >> 8) & 0xFF); // High byte

    return this;
}

PacketBuffer *PacketBuffer::writeString(std::string str)
{
    if (str.size() > 32767)
    {
        throw net_exception("the string is too large!");
    }

    this->writeVarInt(str.size());
    for (size_t i = 0; i < str.size(); i++)
    {
        writeByte(str[i]);
    }

    return this;
}

std::string PacketBuffer::readString()
{
    int length = this->readVarInt();

    if (length > 32767)
    {
        throw net_exception("string too large!");
    }

    std::string str;

    for (int i = 0; i < length; i++)
    {
        str += readByte();
    }

    return str;
}

size_t PacketBuffer::getSize()
{
    return this->vec.size();
}

std::vector<unsigned char> PacketBuffer::toBytes()
{
    return this->vec;
}
