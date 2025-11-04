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
}; // namespace pixel

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
    uint16_t width;
    uint16_t height;
    std::vector<pixel::rgba32> contents;
    rgba32(const engine::memory::const_view input);
    rgba32(const std::string &path);
};

class rgb24
{
  public:
    std::vector<pixel::rgba32> contents;

    rgb24(const engine::memory::const_view
              input); // initialize from png binary data
    rgb24(const std::string &path);
    uint16_t width;
    uint16_t height;
    std::vector<pixel::rgb24> pixels;
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

} // namespace image

extern template class filesystem::cache<engine::image::rgba32>;
