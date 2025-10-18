#include <stdint.h>
#include <vector>
#include <engine/exception.hpp>
#include <engine/memory.hpp>

namespace pixel
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

namespace image
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
    rgba32(const std::string & path);
};

class rgb24
{
    std::vector<pixel::rgba32> contents;

  public:
    rgb24(const engine::memory::const_view
              input); // initialize from png binary data
    rgb24(const std::string & path);
    uint16_t width;
    uint16_t height;
    std::vector<pixel::rgb24> pixels;
};
} // namespace image