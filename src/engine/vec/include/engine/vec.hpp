#pragma once
#include <array>
#include <cstdint>

namespace vec
{
using iscalar = long int;
using fscalar = float;

constexpr float epsilon = 0.0001f;

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
    T &operator[](std::size_t index)
    {
        return *(&x + index);
    }
    const T &operator[](std::size_t index) const
    {
        return *(&x + index);
    }

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
    T x;
    T y;
    T z;
    T w;
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
    vec4 operator/(const fscalar &rhs) const
    {
        return vec4(this->x / rhs, this->y / rhs, this->z / rhs, this->w / rhs);
    }
    const T &operator[](std::size_t index) const
    {
        return *(&x + index);
    }
    T &operator[](std::size_t index)
    {
        return *(&x + index);
    }
    vec4 operator*(const vec4 &rhs) const;
    vec3<T> operator*(const vec3<T> &rhs) const;
};

using fvec2 = vec2<float>;
using fvec3 = vec3<float>;
using fvec4 = vec4<float>;

using i16vec2 = vec2<int16_t>;
using i16vec4 = vec4<int16_t>;
using u16vec2 = vec2<uint16_t>;
using u8vec4 = vec4<uint8_t>;

template <typename T> class mat4;

template <typename T> class mat3
{
  protected:
    std::array<T, 9> indices;

  public:
    mat3()
    {
        for (int i = 0; i < 9; i++)
            indices[i] = (i % 4 == 0) ? (T)1 : (T)0;
    };
    using column = std::array<T, 3>;
    mat3(const column &c0, const column &c1, const column &c2)
        : indices(
              {c0[0], c0[1], c0[2], c1[0], c1[1], c1[2], c2[0], c2[1], c2[2]})
    {
    }
    mat3(const mat4<T> &m);
    // template <typename... L> mat3(L... ts) : indices{ts...} {};
    mat3<T> operator*(const mat3<T> &rhs) const;
    vec3<T> operator*(const vec3<T> &rhs) const;
    const T &operator[](std::size_t index) const
    {
        return indices[index];
    }
    T &operator[](std::size_t index)
    {
        return indices[index];
    }
    bool operator==(const mat3<T> &rhs) const;
};
template <typename T> class mat4
{
  protected:
    std::array<T, 16> indices;

  public:
    mat4()
    {
        for (int i = 0; i < 16; i++)
            indices[i] = (i % 5 == 0) ? (T)1 : (T)0;
    };
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
    // template <typename... L> mat4(L... ts) : indices{ts...} {};
    mat4(T s0,
         T s1,
         T s2,
         T s3,
         T s4,
         T s5,
         T s6,
         T s7,
         T s8,
         T s9,
         T s10,
         T s11,
         T s12,
         T s13,
         T s14,
         T s15)
        : indices({s0,
                   s1,
                   s2,
                   s3,
                   s4,
                   s5,
                   s6,
                   s7,
                   s8,
                   s9,
                   s10,
                   s11,
                   s12,
                   s13,
                   s14,
                   s15})
    {
    }
    mat4<T> operator*(const mat4<T> &rhs) const;
    vec4<T> operator*(const vec4<T> &rhs) const;
    T &operator[](std::size_t index)
    {
        return indices[index];
    }
    const T &operator[](std::size_t index) const
    {
        return indices[index];
    }
    operator mat3<T>() const
    {
        return mat3<T>(indices[0],
                       indices[1],
                       indices[2],
                       indices[4],
                       indices[5],
                       indices[6],
                       indices[8],
                       indices[9],
                       indices[10]);
    }
    bool operator==(const mat4<T> &rhs) const;
};

using fmat4 = mat4<float>;

using fmat3 = mat3<float>;

class transform3
{
  public:
    fvec3 translation;
    fvec4 rotation;
    fvec3 scale;
    transform3(fvec3 _translation = fvec3(0, 0, 0),
               fvec4 _rotation = fvec4(0, 0, 0, 1),
               fvec3 _scale = fvec3(1, 1, 1))
        : translation(_translation), rotation(_rotation), scale(_scale)
    {
    }
};

template <typename T> class cubicspline
{
  public:
    T in_tangent;
    T value;
    T out_tangent;
    cubicspline(T _in_tangent, T _value, T _out_tangent)
        : in_tangent(_in_tangent), value(_value), out_tangent(_out_tangent)
    {
    }
};

fscalar length(const fvec3 &v);
fvec3 normal(const fvec3 &v);
fscalar dot(const fvec3 &a, const fvec3 &b);
fvec3 cross(const fvec3 &a, const fvec3 &b);

fvec3 lerp(const fvec3 &a, const fvec3 &b, float t);
fscalar length(const fvec4 &v);
fvec4 normal(const fvec4 &v);
fscalar dot(const fvec4 &a, const fvec4 &b);
fvec4 slerp(const fvec4 &a, const fvec4 &b, float t);

class fmat4_translation : public fmat4
{
  public:
    fmat4_translation(const fvec3 &translation);
};

class fmat4_rotation : public fmat4
{
    fmat4_rotation(fscalar s, const fvec4 &quaternion);

  public:
    fmat4_rotation(const fvec4 &quaternion);
};

class fmat4_scale : public fmat4
{
  public:
    fmat4_scale(const fvec3 &scale);
};

class fmat4_transform3 : public fmat4
{
  public:
    fmat4_transform3(const transform3 &transform);
    fmat4_transform3(const fmat4_translation &translation,
                     const fmat4_rotation &rotation,
                     const fmat4_scale &scale);
};

}; // namespace vec