#include "progress_writer.hpp"

std::mutex ProgressWriter::mutex;
std::ofstream *ProgressWriter::stream;
bool ProgressWriter::isOpen = false;

void ProgressWriter::open()
{
    std::ofstream *out = new std::ofstream("progress.txt", std::ios::out | std::ios::trunc);
    if (!(*out).good())
    {
        throw std::runtime_error("Failed to open progress.txt");
    }

    stream = out;
    ProgressWriter::isOpen = true;
}

void ProgressWriter::writeIP(std::string str)
{
    mutex.lock();

    if (!ProgressWriter::isOpen)
    {
        ProgressWriter::open();
    }

    if (ProgressWriter::stream == NULL)
        return;

    (*stream) << str << '\n';
    stream->flush();

    delete stream;

    mutex.unlock();
}

void ProgressWriter::close()
{
    mutex.lock();
    if (ProgressWriter::stream == NULL)
        return;

    ProgressWriter::stream->close();
    ProgressWriter::isOpen = false;

    mutex.unlock();
}