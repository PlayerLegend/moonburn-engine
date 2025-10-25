#include <engine/image.hpp>
#include <string.h>
#include <string>
#include <assert.h>
#include <fstream>

#define _libpng_fail_value 0

#ifdef USE_LIBPNG
#include <png.h>

image::rgb24::rgb24(const engine::memory::const_view input)
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

image::rgba32::rgba32(const engine::memory::const_view input)
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

static engine::memory::allocation file_binary(const std::string &file_path)
{
    std::ifstream stream(file_path);

    if (stream)
    {
        engine::memory::allocation result;
        stream.seekg(0, stream.end);
        result.resize(stream.tellg());
        stream.seekg(0, stream.beg);
        stream.read((char *)result.data(), result.size());
        return result;
    }

    throw(errno);
}
image::rgba32::rgba32(const std::string &path) : rgba32(file_binary(path)) {}
image::rgb24::rgb24(const std::string &path) : rgb24(file_binary(path)) {}

#endif // USE_LIBPNG