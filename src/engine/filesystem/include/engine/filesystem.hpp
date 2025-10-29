#pragma once

#include <assert.h>
#include <engine/exception.hpp>
#include <engine/filesystem.hpp>
#include <engine/memory.hpp>
#include <filesystem>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace filesystem
{
namespace exception
{
class base : public engine::exception
{
  public:
    base(const std::string &message) : engine::exception(message) {}
};
class not_found : public filesystem::exception::base
{
  public:
    not_found(const std::string &message) : filesystem::exception::base(message)
    {
    }
};
class wrong_type : public filesystem::exception::base
{
  public:
    wrong_type(const std::string &message)
        : filesystem::exception::base(message)
    {
    }
};
}; // namespace exception
class memory_manager;
class cache
{
    using entry = std::variant<gltf::gltf, image::rgba32, image::rgb24, engine::memory::allocation>;
    using active_t = std::unordered_map<std::string, entry>;
    active_t active;
    std::set<std::string> whitelist;
    const entry &get_entry(const std::string &path);
  public:
    cache(const std::vector<std::string> &roots);
    const engine::memory::allocation &get_binary(const std::string &path);
    const gltf::gltf &get_gltf(const std::string &path);
    const image::rgba32 &get_rgba32(const std::string &path);
    const image::rgb24 &get_rgb24(const std::string &path);
};
} // namespace filesystem