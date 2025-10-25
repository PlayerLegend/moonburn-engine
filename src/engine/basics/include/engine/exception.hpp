#include <exception>
#include <string>

namespace engine
{
class exception : std::exception
{
  public:
    std::string message;
    exception(std::string _message) : message(_message) {}
};
} // namespace engine