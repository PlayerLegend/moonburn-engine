#include <string>
#include <variant>
#include <vector>
#include <unordered_map>

namespace json
{

class value;
class array;
class object;
class number;

class number : public std::variant<long long int, double>
{
    public:
    using base = std::variant<long long int, double>;
    using base::base;
    using base::operator=;
};

class object : public std::unordered_map<std::string, value>
{
    public:
    using base = std::unordered_map<std::string, value>;
    using base::base;
    using base::operator=;
    using base::operator[];
};

class array : public std::vector<value>
{
  public:
    using base = std::vector<value>;
    using base::base;
    using base::operator=;
    using base::operator[];
};

class value : public std::variant<array, number, object, std::string>
{
  public:
    using base = std::variant<array, number, object, std::string>;
    using base::base;
    using base::operator=;
};

}; // namespace json