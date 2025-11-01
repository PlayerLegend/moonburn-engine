#include <engine/gltf.hpp>
#include <iostream>

std::string dump_json_string_from_glb(std::string path)
{
    filesystem::allocation glb_alloc(path);
    gltf::glb glb(glb_alloc);
    std::string result(glb.json.begin, glb.json.end);
    return result;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "dump_glb_json") << " <path-to-glb>" << std::endl;
        return 1;
    }

    const char* path = argv[1];
    try {
        std::string json = dump_json_string_from_glb(path);
        std::cout << json;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }

    return 0;
}

