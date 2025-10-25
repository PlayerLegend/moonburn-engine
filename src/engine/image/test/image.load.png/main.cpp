#include <engine/image.hpp>
#include <assert.h>
#include <iostream>

int main(int argc, char *argv[])
{
    assert(argc == 2);

    image::rgba32 image(argv[1]);

    std::cout << "Success\n";
}