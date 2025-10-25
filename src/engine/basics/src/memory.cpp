#include <engine/memory.hpp>
#include <fstream>

engine::memory::allocation from_file(const std::string &file_path)
{
    std::ifstream stream(file_path);

    if (stream)
    {
        engine::memory::allocation result;
        stream.seekg(0, stream.end);
        result.resize(stream.tellg());
        stream.seekg(0, stream.beg);
        stream.read((char *)result.data(), result.size());
        return result;
    }

    throw(errno);
}