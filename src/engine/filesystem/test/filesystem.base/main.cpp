#include <assert.h>
#include <engine/filesystem.hpp>
#include <filesystem>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <path-to-file>\n";
        return 1;
    }
    std::filesystem::path file_path = argv[1];

    engine::filesystem::whitelist wl(file_path.parent_path().string());
    engine::filesystem::cache_binary cache(wl);
    engine::filesystem::cache_binary::reference ref = cache[file_path.string()];
    if (!ref)
    {
        std::cerr << "Failed to load file: " << file_path.string() << "\n";
        return 2;
    }
    const engine::filesystem::allocation &alloc = *ref;
    std::cout << "Loaded file: " << file_path.string() << " (" << alloc.size()
              << " bytes)\n";

    std::string file_str(alloc.begin(), alloc.end());

    if (file_str != "Test file contents")
    {
        std::cerr << "File contents unexpected: " << file_str << "\n";
        return 3;
    }

    std::cout << "Success for \n" << argv[0] << "\n";

    return 0;
}