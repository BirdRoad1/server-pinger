#include "packetbuffer.hpp"
#include <arpa/inet.h> // For htons()
#include <iostream>
#include <stdexcept>
#include "../exception/net_exception.hpp"
#include <unistd.h>

unsigned char PacketBuffer::readByte() {
    if (fd < 0) {
        if (cursor >= vec.size()) {
            close(fd);
            throw net_exception("Read failed!");
        }

        const unsigned char byte = vec.at(cursor);
        cursor++;

        return byte;
    } else {
        unsigned char bytes[1];

        if (recv(fd, bytes, sizeof(bytes), 0) != 1) {
            close(fd);
            throw net_exception("Failed to read byte from socket");
        }

        return bytes[0];
    }
}

void PacketBuffer::writeByte(unsigned char byte) {
    if (fd < 0) {
        vec.push_back(byte);
        cursor++;
    } else {
        unsigned char bytes[1] = {byte};
        if (send(fd, bytes, sizeof(bytes), 0) != 1) {
            close(fd);
            throw net_exception("Failed to write byte to socket");
        }
    }
}

PacketBuffer *PacketBuffer::writeVarInt(int value) {
    while (true) {
        if ((value & ~SEGMENT_BITS) == 0) {
            writeByte(static_cast<unsigned char>(value));

            return this;
        }

        writeByte(static_cast<unsigned char>((value & SEGMENT_BITS) | CONTINUE_BIT));

        value = static_cast<int>(static_cast<unsigned int>(value) >> 7);
    }
}

int PacketBuffer::readVarInt() {
    int value = 0;
    int position = 0;
    unsigned char currentByte;

    while (true) {
        currentByte = readByte();

        value |= (currentByte & SEGMENT_BITS) << position;

        if ((currentByte & CONTINUE_BIT) == 0)
            break;

        position += 7;

        if (position >= 32) {
            close(fd);
            throw net_exception("VarInt is too big");
        }
    }

    return value;
}

unsigned short PacketBuffer::readUShort() {
    return (readByte() << 8) | readByte();
}

PacketBuffer *PacketBuffer::writeUShort(unsigned short value) {
    uint16_t networkValue = htons(value);

    writeByte(networkValue & 0xFF); // Low byte
    writeByte((networkValue >> 8) & 0xFF); // High byte

    return this;
}

PacketBuffer *PacketBuffer::writeString(const std::string &str) {
    if (str.size() > 32767) {
        throw net_exception("the string is too large!");
    }

    this->writeVarInt(str.size());
    for (const char c: str) {
        writeByte(c);
    }

    return this;
}

std::string PacketBuffer::readString() {
    int length = this->readVarInt();

    if (length > 32767) {
        throw net_exception("string too large!");
    }

    std::string str;

    for (int i = 0; i < length; i++) {
        str += static_cast<char>(readByte());
    }

    return str;
}

size_t PacketBuffer::getSize() const {
    return this->vec.size();
}

std::vector<unsigned char> PacketBuffer::toBytes() {
    return this->vec;
}
