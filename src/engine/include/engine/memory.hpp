#include <vector>
#include <stdint.h>

namespace engine
{
namespace memory
{
class allocation : std::vector<uint8_t>
{
  public:
    using base = std::vector<uint8_t>;
    using base::base;
    using base::iterator;
    using base::begin;
};
class view
{
  public:
    allocation::iterator begin, end;
    view(const allocation & parent): begin(parent.begin()), end(parent.end()){}
};
}; // namespace memory
}; // namespace engine