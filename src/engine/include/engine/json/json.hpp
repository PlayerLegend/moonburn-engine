#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <stdint.h>
#include <format>

namespace json
{

class location;
class value;
class array;
class object;
class number;

typedef long long int number_int;
typedef double number_float;

class location
{
  public:
    uint32_t line = 1;
    uint32_t col = 1;
    std::string filepath;
    location() {}
    location(std::string &_filepath, uint32_t _line, uint32_t _col)
        : line(_line), col(_col), filepath(_filepath)
    {
    }
    location(const std::string &_filepath) : filepath(_filepath) {}
};

class null
{
  public:
    json::location location;
    null(json::location &_location) : location(_location) {}
    value operator[](const uint64_t &index);
};

class exception : std::exception
{
  public:
    json::location location;
    std::string message;
    exception(std::string _filepath,
              unsigned int _line,
              unsigned int _col,
              std::string _message)
        : location(_filepath, _line, _col), message(_message)
    {
    }
    exception(json::location _location, std::string _message)
        : location(_location), message(_message)
    {
    }
    std::string pretty_text()
    {
        return location.filepath + ":" + std::to_string(location.line) + ":" +
               std::to_string(location.col) + ": " + message;
    }
};

class string : public std::string
{
  public:
    json::location location;
    string(const json::location &l) : location(l) {}
    using base = std::string;
    using base::base;
};

class number : public std::variant<number_int, number_float>
{
  public:
    json::location location;
    using base = std::variant<number_int, number_float>;
    using base::base;
    using base::operator=;
    number(number_float n)
    {
        this->base::operator=(n);
    }
    number(number_int n)
    {
        this->base::operator=(n);
    }
    number operator*(number_int rhs) const
    {
        if (std::holds_alternative<number_float>(*this))
            return static_cast<number_float>(*this) * (number_float)rhs;
        else
            return static_cast<number_int>(*this) * rhs;
    }
    number operator*(number_float rhs) const
    {
        if (std::holds_alternative<number_float>(*this))
            return static_cast<number_float>(*this) * rhs;
        else
            return static_cast<number_float>(*this) * rhs;
    }
    number operator*(int rhs) const
    {
        return this->operator*((number_int)rhs);
    }
    number operator-() const
    {
        if (std::holds_alternative<number_float>(*this))
            return -static_cast<number_float>(*this);
        else
            return -static_cast<number_int>(*this);
    }
    explicit operator number_float() const
    {
        if (std::holds_alternative<number_float>(*this))
            return std::get<number_float>(*this);
        else
            return std::get<number_int>(*this);
    }
    explicit operator number_int() const
    {
        if (std::holds_alternative<number_float>(*this))
            return std::get<number_float>(*this);
        else
            return std::get<number_int>(*this);
    }

    number_int convert_int()
    {

        if (std::holds_alternative<number_float>(*this))
            return std::get<number_float>(*this);
        else
            return std::get<number_int>(*this);
    }

    number_float convert_float()
    {
        if (std::holds_alternative<number_float>(*this))
            return std::get<number_float>(*this);
        else
            return std::get<number_int>(*this);
    }

    number_int strict_int()
    {

        if (std::holds_alternative<number_int>(*this))
            return std::get<number_int>(*this);

        throw json::exception(location, "Expected an int, not a float");
    }

    number_float strict_float()
    {
        if (std::holds_alternative<number_float>(*this))
            return std::get<number_float>(*this);
        throw json::exception(location, "Expected a float, not an int");
    }
};

class object : public std::unordered_map<std::string, value>
{
  public:
    json::location location;
    using base = std::unordered_map<std::string, value>;
    using base::base;
    using base::operator=;
    using base::operator[];
};

class array : public std::vector<value>
{
  public:
    json::location location;
    using base = std::vector<value>;
    using base::base;
    using base::operator=;
    using base::operator[];
};

class value : public std::variant<array, number, object, string, null>
{
  public:
    json::location location;
    using base = std::variant<array, number, object, string, null>;
    using base::base;
    using base::operator=;

    value operator[](const std::string &index)
    {
        if (std::holds_alternative<object>(*this))
            return std::get<object>(*this)[index];
        return json::null(location);
    }
    value operator[](const uint64_t &index)
    {
        if (std::holds_alternative<array>(*this))
            return std::get<array>(*this)[index];
        return json::null(location);
    }
    bool operator==(const std::string &rhs)
    {
        if (std::holds_alternative<string>(*this))
            return std::get<string>(*this) == rhs;

        return false;
    }

    operator object()
    {
        if (std::holds_alternative<object>(*this))
            return std::get<object>(*this);
        throw json::exception(location, "Expected an object");
    }

    operator array()
    {
        if (std::holds_alternative<array>(*this))
            return std::get<array>(*this);
        throw json::exception(location, "Expected an array");
    }

    operator number()
    {
        if (std::holds_alternative<number>(*this))
            return std::get<number>(*this);
        throw json::exception(location, "Expected a number");
    }

    operator string()
    {
        if (std::holds_alternative<string>(*this))
            return std::get<string>(*this);
        throw json::exception(location, "Expected a string");
    }

    number_int convert_int()
    {
        if (std::holds_alternative<number>(*this))
            return std::get<number>(*this).convert_int();

        throw json::exception(location, "Expected a number");
    }

    number_float convert_float()
    {
        if (std::holds_alternative<number>(*this))
            return std::get<number>(*this).convert_float();

        throw json::exception(location, "Expected a number");
    }

    number_int strict_int()
    {
        if (std::holds_alternative<number>(*this))
            return std::get<number>(*this).strict_int();

        throw json::exception(location, "Expected a number");
    }

    number_float strict_float()
    {
        if (std::holds_alternative<number>(*this))
            return std::get<number>(*this).strict_float();

        throw json::exception(location, "Expected a number");
    }
};

value parse(const std::string &name, const std::string &text);
value parse_file(const std::string &name);

}; // namespace json