#include "test-values.hpp"
#include <cmath>
#include <engine/vec.hpp>
#include <iostream>

bool mat4x4_equals(const vec::fmat4 &lhs, const vec::fmat4 &rhs)
{
    float epsilon = 0.0001f;
    for (int i = 0; i < 16; ++i)
    {
        if (std::abs(lhs[i] - rhs[i]) > epsilon)
        {
            return false;
        }
    }
    return true;
}

// Compare vec4 results component-wise
bool vec4_equals(const vec::fvec4 &lhs, const vec::fvec4 &rhs)
{
    float epsilon = 0.0001f;
    for (int i = 0; i < 4; ++i)
    {
        if (std::abs(lhs[i] - rhs[i]) > epsilon)
            return false;
    }
    return true;
}

void test_mat4_vec4_multiply()
{
    for (const struct mat4_vec4_multiply_test &test : mat4_vec4_multiply_tests)
    {
        vec::fvec4 result = test.a * test.b;
        if (!vec4_equals(result, test.expected))
        {
            std::cerr << "Test failed: mat4 * vec4 multiplication did not produce expected result."
                      << std::endl;
        }
    }
}

void test_mat4_multiply()
{
    for (const struct mat4_mat4_multiply_test &test : mat4_mat4_multiply_tests)
    {
        vec::fmat4 result = test.a * test.b;
        if (!mat4x4_equals(result, test.expected))
        {
            std::cerr << "Test failed: mat4 multiplication did not produce "
                         "expected result."
                      << std::endl;
        }
    }
}

int main()
{
    test_mat4_vec4_multiply();
    test_mat4_multiply();

    std::cout << "Success" << std::endl;
}