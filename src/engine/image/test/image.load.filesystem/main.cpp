#include <engine/image.hpp>
#include <assert.h>
#include <iostream>

int main(int argc, char *argv[])
{
    assert(argc == 2);

    std::string image_path = argv[1];

    std::string image_dir = std::filesystem::path(image_path).parent_path().string();

    filesystem::whitelist whitelist(image_dir);

    engine::image::rgba32_cache cache(whitelist);

    const engine::image::rgba32 & image(*cache[image_path]);

    std::cout << "Image dimensions: " << image.width << "x" << image.height << "\n";

    std::cout << "Success for \n" << argv[0] << "\n";
}