#pragma once
#include <vector>
#include <string>

class PacketBuffer
{

private:
    int fd = -1;
    std::vector<unsigned char> vec;
    size_t cursor = 0;
    static const int SEGMENT_BITS = 0x7F;
    static const int CONTINUE_BIT = 0x80;

public:
    /**
     * Creates a packet buffer that reads/writes to a stream (fd)
     */
    PacketBuffer(int fd)
    {
        this->fd = fd;
    }

    /**
     * Creates a packet buffer that reads/writes to a vector in-memory
     */
    PacketBuffer() {}

    unsigned char readByte();

    void writeByte(unsigned char byte);

    PacketBuffer *writeVarInt(int value);

    int readVarInt();

    unsigned short readUShort();

    PacketBuffer *writeUShort(unsigned short value);

    PacketBuffer *writeString(std::string str);

    std::string readString();

    size_t getSize();

    std::vector<unsigned char> toBytes();
};