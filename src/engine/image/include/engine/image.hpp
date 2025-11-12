#pragma once
#include <engine/exception.hpp>
#include <engine/filesystem.hpp>
#include <engine/memory.hpp>
#include <stdint.h>
#include <vector>

namespace engine::image
{
class exception : engine::exception
{
  public:
    exception(const std::string &_message) : engine::exception(_message) {}
};

class rgba32
{
  public:
    struct pixel
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

  private:
    std::vector<pixel> contents;

  public:
    uint16_t width;
    uint16_t height;
    rgba32(const engine::memory::const_view input);
    rgba32(const std::string &path);
    const pixel *data() const
    {
        return contents.data();
    }
};

class rgb24
{
  public:
    struct pixel
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

  private:
    std::vector<pixel> contents;

  public:
    rgb24(const engine::memory::const_view input);
    rgb24(const std::string &path);
    uint16_t width;
    uint16_t height;
    const pixel *data() const
    {
        return contents.data();
    }
};

} // namespace engine::image

namespace engine::image::cache
{
class rgba32 : public filesystem::cache<image::rgba32>
{
  protected:
    reference load(const std::string &path, std::filesystem::file_time_type) override;
    std::filesystem::file_time_type get_mtime(const std::string &path) override
    {
        return std::filesystem::last_write_time(path);
    }

  public:
    rgba32(class filesystem::whitelist &wl)
        : filesystem::cache<image::rgba32>(wl)
    {
    }
};

} // namespace engine::image::cache

extern template class engine::filesystem::cache<engine::image::rgba32>;
