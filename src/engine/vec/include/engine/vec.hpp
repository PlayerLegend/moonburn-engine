#pragma once
#include <array>

namespace vec
{
using iscalar = long int;
using fscalar = float;

template <typename T> class vec2
{
  public:
    T x;
    T y;
    vec2() : x(0), y(0) {}
    vec2(T _x, T _y) : x(_x), y(_y) {}
    vec2 operator+(const vec2 &rhs) const
    {
        return vec2(this->x + rhs.x, this->y + rhs.y);
    }
    vec2 operator-(const vec2 &rhs) const
    {
        return vec2(this->x - rhs.x, this->y - rhs.y);
    }
    vec2 operator*(const fscalar &rhs) const
    {
        return vec2(this->x * rhs, this->y * rhs);
    }
};

template <typename T> class vec3
{
  public:
    T x;
    T y;
    T z;
    vec3() : x(0), y(0), z(0) {}
    vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}

    vec3 operator+(const vec3 &rhs) const
    {
        return vec3(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z);
    }
    vec3 operator-(const vec3 &rhs) const
    {
        return vec3(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z);
    }
    vec3 operator*(const fscalar &rhs) const
    {
        return vec3(this->x * rhs, this->y * rhs, this->z * rhs);
    }
    vec3 operator/(const fscalar &rhs) const
    {
        return vec3(this->x / rhs, this->y / rhs, this->z / rhs);
    }

    vec3 normal() const;
    vec3 cross(const vec3 &other) const;
    float dot(const vec3 &other) const;
    float length() const;

    class decompose
    {
      public:
        vec3<T> normal;
        T distance;
        decompose(vec3<T>);
    };
};

template <typename T> class vec4
{
  public:
    union
    {
        struct
        {
            T x;
            T y;
            T z;
            T w;
        };
        std::array<T, 4> indices;
    };
    vec4() : x(0), y(0), z(0), w(0) {}
    vec4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}
    vec4 operator+(const vec4 &rhs) const
    {
        return vec4(this->x + rhs.x,
                    this->y + rhs.y,
                    this->z + rhs.z,
                    this->w + rhs.w);
    }
    vec4 operator-(const vec4 &rhs) const
    {
        return vec4(this->x - rhs.x,
                    this->y - rhs.y,
                    this->z - rhs.z,
                    this->w - rhs.w);
    }
    vec4 operator*(const fscalar &rhs) const
    {
        return vec4(this->x * rhs, this->y * rhs, this->z * rhs, this->w * rhs);
    }
    T operator[](std::size_t index) const
    {
        return indices[index];
    }
    vec4 operator*(const vec4 &rhs) const;
    vec3<T> operator*(const vec3<T> &rhs) const;
};

using fvec3 = vec3<float>;
using fvec4 = vec4<float>;

template <typename T> class mat4
{
  protected:
    std::array<T, 16> indices;

  public:
    mat4() {};
    using column = std::array<T, 4>;
    mat4(const column &c0, const column &c1, const column &c2, const column &c3)
        : indices({c0[0],
                   c0[1],
                   c0[2],
                   c0[3],
                   c1[0],
                   c1[1],
                   c1[2],
                   c1[3],
                   c2[0],
                   c2[1],
                   c2[2],
                   c2[3],
                   c3[0],
                   c3[1],
                   c3[2],
                   c3[3]})
    {
    }
    template <typename... L> mat4(L... ts) : indices{ts...} {};
    mat4<T> operator*(const mat4<T> &rhs) const;
    vec4<T> operator*(const vec4<T> &rhs) const;
    T operator[](std::size_t index) const
    {
        return indices[index];
    }
};

using fmat4 = mat4<float>;

class transform3
{
  public:
    fvec3 translation;
    fvec4 rotation;
    fvec3 scale;
};

}; // namespace vec