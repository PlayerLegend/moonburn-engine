#include <stdint.h>
#include <vector>
#pragma once

namespace engine
{
namespace memory
{
using allocation = std::vector<uint8_t>;

class view
{
  public:
    allocation::iterator begin, end;
    view(allocation &parent) : begin(parent.begin()), end(parent.end()) {}
};
class const_view
{
  public:
    allocation::const_iterator begin, end;
    const_view() {}
    const_view(const allocation &parent)
        : begin(parent.begin()), end(parent.end())
    {
    }
    const_view(const uint8_t *data, size_t size) : begin(data), end(data + size)
    {
    }
    const_view(allocation::const_iterator _begin,
               allocation::const_iterator _end)
        : begin(_begin), end(_end)
    {
    }
    size_t size() const
    {
        return end - begin;
    }
    bool contains(const const_view &other) const;
};
}; // namespace memory
}; // namespace engine