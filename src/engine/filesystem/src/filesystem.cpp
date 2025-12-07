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

void filesystem::whitelist::add(const std::string &name,
                                const std::string &absolute)
{
    (*this)[name] = std::filesystem::absolute(absolute).string();
}

void filesystem::whitelist::add_recursive(const std::string &root)
{
    std::filesystem::path root_path = root;

    for (auto &p : std::filesystem::recursive_directory_iterator(root_path))
    {
        emplace(std::filesystem::relative(p.path(), root_path)
                          .lexically_normal()
                          .string(),
                      std::filesystem::absolute(p.path()).string());
    }
}

} // namespace engine::filesystem