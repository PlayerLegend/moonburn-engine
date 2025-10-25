#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <stdint.h>
#include <engine/exception.hpp>
#include <engine/memory.hpp>
#endif

namespace json
{

class location;
class value;
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
    null() {};
    value operator[](const uint64_t &index);
};

class exception : engine::exception
{
  public:
    json::location location;
    exception(std::string _filepath,
              unsigned int _line,
              unsigned int _col,
              std::string _message)
        : engine::exception(_message), location(_filepath, _line, _col)
    {
    }
    exception(json::location _location, std::string _message)
        : engine::exception(_message), location(_location)
    {
    }
    std::string pretty_text()
    {
        return location.filepath + ":" + std::to_string(location.line) + ":" +
               std::to_string(location.col) + ": " + message;
    }
};

using string = std::string;

class number
{
    std::variant<number_int, number_float> contents;

  public:
    json::location location;
    number(number_float n)
    {
        contents = n;
    }
    number(number_int n)
    {
        contents = n;
    }
    number operator*(number_int rhs) const
    {
        if (std::holds_alternative<number_float>(this->contents))
            return std::get<number_float>(this->contents) * (number_float)rhs;
        else
            return std::get<number_int>(this->contents) * (number_int)rhs;
    }
    number operator*(number_float rhs) const
    {
        if (std::holds_alternative<number_float>(this->contents))
            return std::get<number_float>(this->contents) * rhs;
        else
            return std::get<number_int>(this->contents) * rhs;
    }
    number operator*(int rhs) const
    {
        return this->operator*((number_int)rhs);
    }
    number operator-() const
    {
        if (std::holds_alternative<number_float>(this->contents))
            return -std::get<number_float>(this->contents);
        else
            return -std::get<number_int>(this->contents);
    }
    explicit operator number_float() const
    {
        if (std::holds_alternative<number_float>(this->contents))
            return std::get<number_float>(this->contents);
        else
            return std::get<number_int>(this->contents);
    }
    explicit operator number_int() const
    {
        if (std::holds_alternative<number_float>(this->contents))
            return std::get<number_float>(this->contents);
        else
            return std::get<number_int>(this->contents);
    }

    number_int as_int() const
    {

        if (std::holds_alternative<number_float>(this->contents))
            return std::get<number_float>(this->contents);
        else
            return std::get<number_int>(this->contents);
    }

    number_float as_float() const
    {
        if (std::holds_alternative<number_float>(this->contents))
            return std::get<number_float>(this->contents);
        else
            return std::get<number_int>(this->contents);
    }

    number_int strict_int() const
    {

        if (std::holds_alternative<number_int>(this->contents))
            return std::get<number_int>(this->contents);

        throw json::exception(location, "Expected an int, not a float");
    }

    number_float strict_float() const
    {
        if (std::holds_alternative<number_float>(this->contents))
            return std::get<number_float>(this->contents);
        throw json::exception(location, "Expected a float, not an int");
    }
};

using object = std::unordered_map<std::string, value>;

using array = std::vector<value>;

class value
{
    std::variant<array, number, object, string, null> contents = null();

  public:
    json::location location;

    bool operator==(const string &str) const
    {

        if (std::holds_alternative<string>(this->contents))
            return std::get<string>(this->contents) == str;
        return false;
    }

    value operator=(const array &&other)
    {
        this->contents = std::move(other);
        return *this;
    };

    value operator=(const array &other)
    {
        this->contents = other;
        return *this;
    };

    value operator=(const number &other)
    {
        this->contents = other;
        return *this;
    };
    value operator=(const object &other)
    {
        this->contents = other;
        return *this;
    };
    value operator=(const string &other)
    {
        this->contents = other;
        return *this;
    };

    value operator=(const null &other)
    {
        this->contents = other;
        return *this;
    };

    value operator=(const value &other)
    {
        if (this != &other)
            *this = other;
        return *this;
    };
    value() {}
    value(const array &&other) : contents(std::move(other)) {}
    value(const string &&other) : contents(std::move(other)) {}
    value(const object &&other) : contents(std::move(other)) {}
    value(const array &other) : contents(other) {}
    value(const string &other) : contents(other) {}
    value(const object &other) : contents(other) {}
    value(const number &other) : contents(other) {}
    value(const null &other) : contents(other) {}
    value operator[](const std::string &index)
    {
        if (std::holds_alternative<object>(this->contents))
            return std::get<object>(this->contents)[index];
        return json::null(location);
    }
    value operator[](const uint64_t &index)
    {
        if (std::holds_alternative<array>(this->contents))
            return std::get<array>(this->contents)[index];
        return json::null(location);
    }

    operator const object&() const
    {
        if (std::holds_alternative<object>(this->contents))
            return std::get<object>(this->contents);
        throw json::exception(location, "Expected an object");
    }

    operator const array&() const
    {
        if (std::holds_alternative<array>(this->contents))
            return std::get<array>(this->contents);
        throw json::exception(location, "Expected an array");
    }

    operator const number&() const
    {
        if (std::holds_alternative<number>(this->contents))
            return std::get<number>(this->contents);
        throw json::exception(location, "Expected a number");
    }

    operator const string&() const
    {
        if (std::holds_alternative<string>(this->contents))
            return std::get<string>(this->contents);
        throw json::exception(location, "Expected a string");
    }

    number_int as_int() const
    {
        if (std::holds_alternative<number>(this->contents))
            return std::get<number>(this->contents).as_int();

        throw json::exception(location, "Expected a number");
    }

    number_float as_float() const
    {
        if (std::holds_alternative<number>(this->contents))
            return std::get<number>(this->contents).as_float();

        throw json::exception(location, "Expected a number");
    }

    number_int strict_int() const
    {
        if (std::holds_alternative<number>(this->contents))
            return std::get<number>(this->contents).strict_int();

        throw json::exception(location, "Expected a number");
    }

    number_float strict_float() const
    {
        if (std::holds_alternative<number>(this->contents))
            return std::get<number>(this->contents).strict_float();

        throw json::exception(location, "Expected a number");
    }
};

value parse(const std::string &name, const std::string &text);
value parse_memory(const std::string &name, engine::memory::const_view input);
value parse_file(const std::string &name);
}; // namespace json