#include <vector>
#include <stdint.h>

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
    const_view(const allocation &parent)
        : begin(parent.begin()), end(parent.end())
    {
    }
};
}; // namespace memory
}; // namespace engine