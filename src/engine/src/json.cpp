#include <fstream>
#include <engine/json/json.hpp>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>
#include <string>

class state
{
  public:
    std::string::const_iterator point;
    const std::string::const_iterator end;
    json::location location;
    state(const std::string &_filename, const std::string &input)
        : point(input.begin()), end(input.end()), location(_filename)
    {
    }
    bool skip_whitespace()
    {
        while (point < end && isspace(*point))
            next();
        return point < end;
    }
    char peek()
    {
        return *point;
    }
    char next()
    {
        char c = *point;
        point++;

        if (c == '\n')
        {
            location.line++;
            location.col = 1;
        }
        else
        {
            location.col++;
        }

        return c;
    }
    size_t remaining()
    {
        return end - point;
    }
    json::exception get_exception(const std::string &message)
    {
        return json::exception(location, message);
    }
};

static json::value parse_value(state &state);

static unsigned char parse_hex_char(state &state)
{
    unsigned char digit[2];

    if (state.remaining() < 2)
        throw state.get_exception(
            "Too few characters remain to read a hex byte");

    char hex[2] = {state.next(), state.next()};

    if (!isxdigit(hex[0]) || !isxdigit(hex[1]))
        throw state.get_exception(
            "Attempted to interpret non-hex characters as hex");

    for (size_t i = 0; i < 2; i++)
    {
        digit[i] = hex[i];
        if (digit[i] >= 'A' && digit[i] <= 'F')
        {
            digit[i] = (10 + digit[i] - 'A');
        }
        else if (digit[i] >= 'a' && digit[i] <= 'f')
        {
            digit[i] = (10 + digit[i] - 'a');
        }
        else
        {
            digit[i] -= '0';
        }
    }

    return 16 * digit[0] + digit[1];
}

static json::string parse_string(state &state)
{
    if (state.next() != '"')
        throw state.get_exception("Expected a string");

    json::string result;

    bool escape = false;

    while (state.point < state.end)
    {
        char c = state.next();
        if (escape)
        {
            switch (c)
            {
            case '"':
            case '\\':
            case '/':
                result.push_back(c);
                break;

            case 'b':
                result.push_back('\b');
                break;

            case 'f':
                result.push_back('\f');
                break;

            case 'n':
                result.push_back('\n');
                break;

            case 'r':
                result.push_back('\r');
                break;

            case 't':
                result.push_back('\t');
                break;
            case 'u':
                if (state.end - state.point < 4)
                    throw state.get_exception(
                        "Started a unicode sequence with fewer than four "
                        "characters remaining");

                result.push_back(parse_hex_char(state));
                result.push_back(parse_hex_char(state));
                break;
            default:
                throw state.get_exception("Invalid escape character");
            }

            escape = false;

            continue;
        }
        else if (c == '\\')
        {
            escape = true;
            continue;
        }
        else if (c == '"')
        {
            return result;
        }
        else
        {
            result.push_back(c);
        }
    }

    throw state.get_exception("Input ended while parsing string");
}

static json::number parse_octal(state &state)
{
    if (state.next() != '0')
        throw state.get_exception("Expected an octal number");

    json::number_int result = 0;

    while (state.point < state.end)
    {
        char c = state.peek();

        if (c < '0' || c >= '8')
            break;

        state.next();

        unsigned char digit = c - '0';

        result = digit + result * 8;
    }

    return result;
}

json::number_int parse_digits(state &state)
{
    uint64_t result = 0;

    while (state.point < state.end)
    {
        char c = state.peek();

        // std::cout << "Digit char: " << c << "\n";

        if (!isdigit(c))
            break;

        state.next();

        unsigned char digit = c - '0';

        result = digit + result * 10;
    }

    return result;
}

double parse_fraction(state &state)
{
    std::string::const_iterator digits_begin = state.point;
    uint64_t numerator = parse_digits(state);
    std::string::const_iterator digits_end = state.point;

    uint64_t denominator = 1;

    while (digits_begin < digits_end)
    {
        denominator *= 10;
        digits_begin++;
    }

    assert(numerator < denominator);

    return (double)numerator / (double)denominator;
}

static json::number parse_number(state &state)
{
    bool is_negative = state.peek() == '-';

    if (is_negative)
        state.next();

    json::number result = parse_digits(state);

    int64_t exponent = 0;

    if (state.point < state.end)
    {
        char c = state.peek();

        if (c == '.')
        {
            state.next();
            json::number_float float_part = parse_fraction(state);
            result = (json::number_float)result + float_part;
        }
    }

    if (state.point < state.end)
    {
        char c = state.peek();
        if (c == 'e' || c == 'E')
        {
            state.next();
            char sign = state.peek();
            if (sign == '-' || sign == '+')
                state.next();
            exponent = parse_digits(state);
            if (sign == '-')
            {
                double result_f = static_cast<json::number_float>(result);

                while (exponent > 0)
                {
                    result_f /= 10;
                    exponent--;
                }

                result = result_f;
            }
            else
            {
                while (exponent > 0)
                {
                    result = result * 10;
                    exponent--;
                }
            }
        }
    }

    if (is_negative)
        result = -result;

    return result;
}

static json::array parse_array(state &state)
{
    if (state.next() != '[')
        throw state.get_exception("Expected a JSON array");

    json::array result;

    bool expect_value = true;

    while (state.point < state.end)
    {
        char c = state.peek();

        if (isspace(c))
        {
            state.next();
            continue;
        }

        if (c == ',')
        {
            if (expect_value)
                throw state.get_exception("Expected a value here, not ','");

            expect_value = true;
            state.next();

            continue;
        }

        if (c == ']')
        {
            if (expect_value && !result.empty())
                throw state.get_exception(
                    "Dangling ',' at the end of the array");

            state.next();
            return result;
        }

        result.push_back(parse_value(state));

        expect_value = false;
    }

    throw state.get_exception("Input ended while reading array");
}

static json::object parse_object(state &state)
{
    if (state.next() != '{')
        throw state.get_exception("Expected a JSON object");

    json::object result;
    state.skip_whitespace();
    if (state.peek() == '}')
        return result;

    while (state.skip_whitespace())
    {
        std::string key = parse_string(state);

        if (!state.skip_whitespace() || state.next() != ':')
            throw state.get_exception("Expected a ':' here");

        result.emplace(key, parse_value(state));

        if (!state.skip_whitespace())
            throw state.get_exception(
                "Input ended while seeking the next line in an object");

        char c = state.next();

        if (c == '}')
            return result;

        if (c != ',')
            throw state.get_exception("Unexpected character in object");
    }

    throw state.get_exception("Input ended while reading an object");
}

static json::value parse_value(state &state)
{

    while (state.skip_whitespace())
    {
        char c = state.peek();

        if (c == '{')
            return parse_object(state);
        if (c == '[')
            return parse_array(state);
        if (c == '"')
            return parse_string(state);
        if (c == '0')
            return parse_octal(state);
        if (isdigit(c) || c == '-')
        {
            return parse_number(state);
        }

        throw state.get_exception("Unexpected character");
    }

    throw state.get_exception("Empty input");
}

json::value json::parse(const std::string &name, const std::string &input)
{
    state state(name, input);

    return parse_value(state);
}

static std::string file_string(const std::string &file_path)
{
    std::ifstream stream(file_path);

    if (stream)
    {
        std::string result;
        stream.seekg(0, stream.end);
        result.resize(stream.tellg());
        stream.seekg(0, stream.beg);
        stream.read(&result[0], result.size());
        return result;
    }

    throw(errno);
}

json::value json::parse_file(const std::string &name)
{
    return parse(name, file_string(name));
}