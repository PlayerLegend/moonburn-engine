#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <stdint.h>
#include <engine/exception.hpp>

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
    null(){};
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

class string
{
    std::string parent;

  public:
    json::location location;
    string(const json::location &l) : location(l) {}
    operator std::string()
    {
        return parent;
    }
    void push_back(char c)
    {
        parent.push_back(c);
    }
    bool operator==(const std::string &rhs) const
    {
        return parent == rhs;
    }
    bool operator==(const string &rhs) const
    {
        return parent == rhs.parent;
    }
};

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
            return std::get<number_float>(this->contents) * rhs;
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

class object
{
    std::unordered_map<std::string, value> contents;

  public:
    json::location location;
    const value &operator[](const std::string &index);
    void emplace(const std::string &index, json::value &&value);
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
    bool operator==(const std::string &rhs) const
    {
        if (std::holds_alternative<string>(this->contents))
            return std::get<string>(this->contents) == rhs;

        return false;
    }

    operator object()
    {
        if (std::holds_alternative<object>(this->contents))
            return std::get<object>(this->contents);
        throw json::exception(location, "Expected an object");
    }

    operator array()
    {
        if (std::holds_alternative<array>(this->contents))
            return std::get<array>(this->contents);
        throw json::exception(location, "Expected an array");
    }

    operator number()
    {
        if (std::holds_alternative<number>(this->contents))
            return std::get<number>(this->contents);
        throw json::exception(location, "Expected a number");
    }

    operator string()
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
value parse_file(const std::string &name);
}; // namespace json