#include <engine/memory.hpp>
#include <filesystem>
#include <fstream>

bool engine::memory::const_view::contains(const const_view &other) const
{
    if (end < begin)
        return false;
    if (other.end < other.begin)
        return false;
    return (other.begin >= begin) && (other.end <= end);
}