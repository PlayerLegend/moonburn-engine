#include <string>
#include <variant>
#include <vector>
#include <unordered_map>

namespace json
{

class value;
class array;
class object;

class number
{
  public:
    number(long long int val) : as_int(val), as_float(val) {}
    number(double val) : as_int(val), as_float(val) {}
    void operator=(const long long int &other)
    {
        as_int = other;
        as_float = other;
    }
    void operator=(const unsigned long long &other)
    {
        as_int = other;
        as_float = other;
    }
    void operator=(const double &other)
    {
        as_int = other;
        as_float = other;
    }
    long long int as_int;
    double as_float;
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