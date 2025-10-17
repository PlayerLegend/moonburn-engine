#include <stdint.h>
#include <vector>
#include <engine/exception.hpp>

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

class rgba32 : std::vector<pixel::rgba32>
{
  public:
    rgba32(const uint8_t * begin, const uint8_t * end);
    uint16_t width;
    uint16_t height;
};

class rgb24 : std::vector<pixel::rgb24>
{
  public:
    rgb24(const std::vector<uint8_t> &input); // initialize from png binary data
    uint16_t width;
    uint16_t height;
    std::vector<pixel::rgb24> pixels;
};
} // namespace image