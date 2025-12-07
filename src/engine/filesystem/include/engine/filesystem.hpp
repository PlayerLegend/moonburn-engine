#pragma once

#include <assert.h>
#include <engine/exception.hpp>
#include <engine/filesystem.hpp>
#include <engine/memory.hpp>
#include <filesystem>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace engine::filesystem::exception
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
}; // namespace engine::filesystem::exception
namespace engine::filesystem
{
class allocation : public engine::memory::allocation
{
  public:
    allocation(const std::string &path);
};

class whitelist : std::unordered_map<std::string, std::string>
{
    using base = std::unordered_map<std::string, std::string>;

  public:
    whitelist() {}
    whitelist(const std::string &root);
    void add(const std::string &name, const std::string &absolute);
    void add_recursive(const std::string &root);
    using base::begin;
    using base::end;
    using base::find;
};

template <typename T, typename... L> class cache
{
  public:
    using mtime = std::filesystem::file_time_type;
    class file
    {
      protected:
        T contents;

      public:
        mtime last_modified;
        std::string path;
        file(const std::string &_path, mtime mtime, L... args)
            : contents(_path, args...), last_modified(mtime), path(_path)
        {
        }
        operator const T &()
        {
            return contents;
        }
    };

    using reference = std::shared_ptr<file>;
    using map = std::unordered_map<std::string, reference>;

  protected:
    const class whitelist &whitelist;
    map contents;
    std::mutex mutex;

    virtual std::filesystem::file_time_type
    get_mtime(const std::string &path) = 0;
    virtual reference load(const std::string &path_rel,
                           const std::string &path_abs,
                           std::filesystem::file_time_type) = 0;

  public:
    cache(const class whitelist &wl) : whitelist(wl) {}

    reference operator[](const std::string &_path)
    {
        std::lock_guard<std::mutex> lock(mutex);
        const auto path = whitelist.find(_path);
        if (path == whitelist.end())
            throw filesystem::exception::not_found("Path not in whitelist: " +
                                                   _path);

        mtime mtime = get_mtime(path->second);
        typename map::iterator it = contents.find(_path);

        if (it == contents.end() || it->second->last_modified < mtime)
            return contents[_path] = load(path->first, path->second, mtime);
        return it->second;
    };

    bool contains(const std::string &path)
    {
        return whitelist.find(path) != whitelist.end();
    }
};

class cache_binary : public cache<filesystem::allocation>
{
  protected:
    reference load(const std::string &path_rel,
                   const std::string &path_abs,
                   std::filesystem::file_time_type mtime) override
    {
        return std::make_shared<cache_binary::file>(path_abs, mtime);
    }

    std::filesystem::file_time_type get_mtime(const std::string &path) override
    {
        return std::filesystem::last_write_time(path);
    }

  public:
    cache_binary(const class whitelist &wl) : cache<filesystem::allocation>(wl)
    {
    }
};

}; // namespace engine::filesystem