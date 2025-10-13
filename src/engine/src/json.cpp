#include "json.hpp"
#include <ctype.h>

class state
{
  public:
    std::string::const_iterator point;
    const std::string::const_iterator end;
    unsigned int line;
    unsigned int col;
};

static json::object parse_object(state &state);
static std::string parse_string(state &state)
{
    if (*state.point != '"')
    {
        throw json::exception(
            state.line,
            state.col,
            "Expected a '\"' character during string parsing");
    }
}
static json::number parse_number(state &state);
static json::array parse_array(state &state);
static json::value parse_value(state &state)
{
    while (state.point < state.end)
    {
        if (isspace(*state.point))
        {
            state.point++;
            continue;
        }

        if (*state.point == '{')
        {
            return parse_object(state);
        }
        if (*state.point == '[')
        {
            return parse_array(state);
        }
        if (*state.point == '"')
        {
            return parse_string(state);
        }
        if (isdigit(*state.point) || *state.point == '-')
        {
            return parse_number(state);
        }

        throw json::exception(state.line, state.col, "Unexpected character");
    }
    
    throw json::exception(state.line, state.col, "Empty input");
}

json::value json::parse(const std::string &input)
{
    std::string::const_iterator point = input.begin();
    std::string::const_iterator end = input.end();
}