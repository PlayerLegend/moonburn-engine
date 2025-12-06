#include <engine/filesystem.hpp>
#include <fstream>

namespace engine::filesystem
{
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

filesystem::allocation::allocation(const std::string &file_path)
    : engine::memory::allocation(from_file(file_path))
{
}

filesystem::whitelist::whitelist(const std::string &root)
{
    add_recursive(root);
}

void filesystem::whitelist::add(const std::string &path)
{
    paths.insert(std::filesystem::absolute(path).string());
}

void filesystem::whitelist::add_recursive(const std::string &root)
{
    for (auto &p : std::filesystem::recursive_directory_iterator(root))
    {
        paths.insert(std::filesystem::absolute(p.path()).string());
    }
}

bool filesystem::whitelist::contains(const std::string &path) const
{
    return paths.find(std::filesystem::absolute(path).string()) != paths.end();
}
} // namespace engine::filesystem