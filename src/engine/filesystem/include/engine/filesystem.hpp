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

class allocation : public engine::memory::allocation
{
  public:
    allocation(const std::string &path);
};

class whitelist
{
    std::mutex mutex;
    std::unordered_set<std::string> paths;

  public:
    whitelist() {}
    whitelist(const std::string &root);
    void add(const std::string &path);
    void add_recursive(const std::string &root);
    bool contains(const std::string &path);
};

template <typename T, typename... L> class file
{
  protected:
    std::filesystem::file_time_type last_modified;
    T contents;

  public:
    file(const std::string &path, L... args) : contents(path, args...) {}
    operator const T &()
    {
        return contents;
    }
};

template <typename T, typename... L> class cache
{
  public:
    using reference = std::shared_ptr<file<T, L...>>;
    using map = std::unordered_map<std::string, reference>;

  protected:
    class whitelist &whitelist;
    map contents;
    std::mutex mutex;

    virtual reference load(const std::string &path) = 0;

  public:
    cache(class whitelist &wl) : whitelist(wl) {}

    reference operator[](const std::string &_path)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (!whitelist.contains(_path))
            throw filesystem::exception::not_found("Path not in whitelist: " +
                                                   _path);
        typename map::iterator it = contents.find(_path);
        if (it == contents.end())
        {
            reference ref = load(_path);
            contents.emplace(_path, ref);
            return ref;
        }
        return it->second;
    };

    using cache_binary = cache<engine::memory::allocation>;
};
}; // namespace filesystem