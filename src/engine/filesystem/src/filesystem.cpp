#include <engine/filesystem.hpp>
#define FLAT_INCLUDES
#include <filesystem>
#include <engine/vec.hpp>
#include <engine/gltf.hpp>
#include <engine/image.hpp>

void add_paths_to_set_recursive(std::set<std::string> &set,
                                const std::string &root)
{
    for (const std::filesystem::directory_entry &entry :
         std::filesystem::recursive_directory_iterator(root))
    {
        if (entry.is_regular_file() || entry.is_symlink())
            set.insert(std::filesystem::canonical(entry.path()).string());
    }
}
filesystem::cache::cache(const std::vector<std::string> &roots)
{
    for (const std::string &root : roots)
        add_paths_to_set_recursive(this->whitelist, root);
}

const filesystem::cache::entry &filesystem::cache::get_entry(const std::string &_path)
{
    std::filesystem::path path = std::filesystem::canonical(_path);

    if (this->whitelist.find(path.string()) != this->whitelist.end())
    {
        active_t::iterator it = this->active.find(path.string());

        if (it != this->active.end())
        {
            return it->second;
        }
    }
    throw filesystem::exception::not_found("File not found in filesystem: " +
                                           path.string());
}

const engine::memory::allocation &
filesystem::cache::get_binary(const std::string &_path)
{
    std::string path = std::filesystem::canonical(_path).string();

    if (this->whitelist.find(path) != this->whitelist.end())
    {
        active_t::iterator it = this->active.find(path);

        if (it != this->active.end())
        {
            if (std::holds_alternative<engine::memory::allocation>(it->second))
            {
                return std::get<engine::memory::allocation>(it->second);
            }
            else
            {
                throw filesystem::exception::wrong_type(
                    "File is not a binary file: " + path);
            }
        }
    }

    throw filesystem::exception::not_found("File not found in filesystem: " +
                                           path);
}
