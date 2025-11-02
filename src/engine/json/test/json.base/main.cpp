#include <assert.h>
#include <engine/json.hpp>
#include <iostream>

void test_json_1(const std::string &file_path)
{
    json::value root = json::parse_file(file_path);

    json::object key1 = root["key1"];
    json::number ababab = root["ababab"];
    json::number asdf = root["ababab"];
    json::array asdf2 = root["asdf2"];
    json::object nest1 = root["nest1"];

    assert(asdf2.size() == 3);
    assert(asdf2[0].strict_int() == 5);
    assert(asdf2[1] == "a2");
    assert(asdf2[2].strict_int() == 9);

    assert(nest1["nest2"].strict_float() == 3.14);
    assert(nest1["nest3"] == "aaa");
    assert(nest1["nest4"] == "abc");
}

void test_json_2(const std::string &file_path)
{
    json::value root = json::parse_file(file_path);
}

int main(int argc, char *argv[])
{
    assert(argc == 3);

    test_json_1(argv[1]);
    test_json_2(argv[2]);

    std::cout << "Success\n";
}