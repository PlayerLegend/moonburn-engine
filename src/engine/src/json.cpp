#include "json.hpp"
#include <ctype.h>

static json::object parse_object(std::string::const_iterator &point,
                                 std::string::const_iterator end);
static std::string parse_string(std::string::const_iterator &point,
                                std::string::const_iterator end);
static json::number parse_number(std::string::const_iterator &point,
                                 std::string::const_iterator end);
static json::array parse_array(std::string::const_iterator &point,
                               std::string::const_iterator end);
static json::value parse_value(std::string::const_iterator &point,
                               std::string::const_iterator end)
{
    while (point < end)
    {
        if (isspace(*point))
        {
            point++;
            continue;
        }

        if (*point == '{')
        {
            return parse_object(point, end);
        }
        if (*point == '[')
        {
            return parse_array(point, end);
        }
        if (*point == '"')
        {
            return parse_string(point, end);
        }
        if (isdigit(*point) || *point == '-')
        {
            return parse_number(point, end);
        }

        
    }
}

json::value json::parse(const std::string &input)
{
    std::string::const_iterator point = input.begin();
    std::string::const_iterator end = input.end();
}