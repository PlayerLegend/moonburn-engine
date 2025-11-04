#pragma once
#include <engine/exception.hpp>
#include <engine/filesystem.hpp>
#include <engine/memory.hpp>
#include <stdint.h>
#include <vector>

namespace engine::pixel
{
struct rgba32
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct rgb24
{
    uint8_t r, g, b;
};
}; // namespace engine::pixel

namespace engine::image
{
class exception : engine::exception
{
  public:
    exception(const std::string &_message) : engine::exception(_message) {}
};

class rgba32
{
    std::vector<pixel::rgba32> contents;

  public:
    uint16_t width;
    uint16_t height;
    rgba32(const engine::memory::const_view input);
    rgba32(const std::string &path);
    const engine::pixel::rgba32 *data() const
    {
        return contents.data();
    }
};

class rgb24
{
    std::vector<pixel::rgba32> contents;

  public:
    rgb24(const engine::memory::const_view
              input); // initialize from png binary data
    rgb24(const std::string &path);
    uint16_t width;
    uint16_t height;
    std::vector<pixel::rgb24> pixels;
    const engine::pixel::rgb24 *data() const
    {
        return pixels.data();
    }
};

using rgba32_file = filesystem::file<image::rgba32>;

class rgba32_cache : public filesystem::cache<image::rgba32>
{
  protected:
    reference load(const std::string &path) override;

  public:
    rgba32_cache(class filesystem::whitelist &wl)
        : filesystem::cache<image::rgba32>(wl)
    {
    }
};

} // namespace engine::image

extern template class filesystem::cache<engine::image::rgba32>;
