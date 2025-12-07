#include <assert.h>
#include <engine/image.hpp>
#include <string.h>
#include <string>

#define _libpng_fail_value 0

#ifdef USE_LIBPNG
#include <png.h>

namespace engine::image
{
rgb24::rgb24(const engine::memory::const_view input)
{
    png_image image;

    memset(&image, 0, sizeof(image));

    image.version = PNG_IMAGE_VERSION;

    if (_libpng_fail_value ==
        png_image_begin_read_from_memory(&image,
                                         &input.begin[0],
                                         input.end - input.begin))

    {
        std::string message = image.message;
        throw image::exception(
            "png_image_begin_read_from_memory returned error: " + message);
    }

    image.format = PNG_FORMAT_RGB;

    size_t size = PNG_IMAGE_SIZE(image);

    assert(size % 3 == 0);

    contents.resize(size / 3);

    if (_libpng_fail_value ==
        png_image_finish_read(&image, NULL, contents.data(), 0, NULL))
    {
        throw image::exception("Parsing a PNG image failed");
    }

    width = image.width;
    height = image.height;
}

rgba32::rgba32(const engine::memory::const_view input)
{
    png_image image;

    memset(&image, 0, sizeof(image));

    image.version = PNG_IMAGE_VERSION;

    if (_libpng_fail_value ==
        png_image_begin_read_from_memory(&image,
                                         &input.begin[0],
                                         input.end - input.begin))

    {
        std::string message = image.message;
        throw image::exception(
            "png_image_begin_read_from_memory returned error: " + message);
    }

    image.format = PNG_FORMAT_RGBA;

    size_t size = PNG_IMAGE_SIZE(image);

    assert(size % 4 == 0);

    contents.resize(size / 3);

    if (_libpng_fail_value ==
        png_image_finish_read(&image, NULL, contents.data(), 0, NULL))
    {
        throw image::exception("Parsing a PNG image failed");
    }

    width = image.width;
    height = image.height;
}

#endif // USE_LIBPNG

rgba32::rgba32(const std::string &path)
    : rgba32(filesystem::allocation(path)) {};

rgb24::rgb24(const std::string &path) : rgb24(filesystem::allocation(path)) {};

} // namespace engine::image

namespace engine::image::cache
{

cache::rgba32::reference
cache::rgba32::load(const std::string &path_rel,
                    const std::string &path_abs,
                    std::filesystem::file_time_type mtime)
{
    return std::make_shared<engine::image::cache::rgba32::file>(path_abs,
                                                                mtime);
}
} // namespace engine::image::cache

template class engine::filesystem::cache<engine::image::rgba32>;
