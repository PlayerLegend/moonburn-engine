#include <cmath>
#include <engine/vec.hpp>

template <> vec::fmat4 vec::fmat4::operator*(const fmat4 &rhs) const
{
    vec::fmat4 result;
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k)
                sum += this->indices[k * 4 + row] * rhs.indices[col * 4 + k];
            result.indices[col * 4 + row] = sum;
        }
    }
    return result;
}

template <> vec::fvec4 vec::fvec4::operator*(const vec::fvec4 &rhs) const
{
    vec::fvec4 result;
    result.w =
        this->w * rhs.w - this->x * rhs.x - this->y * rhs.y - this->z * rhs.z;
    result.x =
        this->w * rhs.x + this->x * rhs.w + this->y * rhs.z - this->z * rhs.y;
    result.y =
        this->w * rhs.y - this->x * rhs.z + this->y * rhs.w + this->z * rhs.x;
    result.z =
        this->w * rhs.z + this->x * rhs.y - this->y * rhs.x + this->z * rhs.w;
    return result;
}

template <> vec::fvec3 vec::fvec4::operator*(const vec::fvec3 &rhs) const
{
    // Treat rhs as a pure quaternion (x,y,z, w=0)
    vec::fvec4 p(rhs.x, rhs.y, rhs.z, 0.0f);
    // Conjugate (inverse for unit quaternions) of *this
    vec::fvec4 conj(-this->x, -this->y, -this->z, this->w);
    // rotation: q * p * q_conj
    vec::fvec4 rotated = (*this) * p * conj;
    return vec::fvec3(rotated.x, rotated.y, rotated.z);
}

template <> vec::fvec4 vec::fmat4::operator*(const vec::fvec4 &rhs) const
{
    vec::fvec4 result;
    for (int row = 0; row < 4; ++row)
    {
        float sum = 0.0f;
        for (int col = 0; col < 4; ++col)
            sum += (*this)[col * 4 + row] * rhs[col];
        result[row] = sum;
    }
    return result;
}

template <> vec::fvec3 vec::fmat3::operator*(const vec::fvec3 &rhs) const
{
    vec::fvec3 result;
    for (int row = 0; row < 3; ++row)
    {
        float sum = 0.0f;
        for (int col = 0; col < 3; ++col)
            sum += (*this)[col * 3 + row] * rhs[col];
        result[row] = sum;
    }
    return result;
}

vec::fscalar vec::length(const fvec3 &v)
{
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec::fvec3 vec::normal(const fvec3 &v)
{
    float len = length(v);
    if (len > 0.00001f)
    {
        return v / len;
    }
    else
    {
        return fvec3(0.0f, 0.0f, 0.0f);
    }
}

vec::fscalar vec::dot(const fvec3 &a, const fvec3 &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec::fvec3 vec::cross(const fvec3 &a, const fvec3 &b)
{
    return fvec3(a.y * b.z - a.z * b.y,
                 a.z * b.x - a.x * b.z,
                 a.x * b.y - a.y * b.x);
}

vec::fvec3 vec::lerp(const fvec3 &a, const fvec3 &b, float t)
{
    return a * (1.0f - t) + b * t;
}

template <> vec::fvec3::decompose::decompose(vec::fvec3 v)
{
    distance = vec::length(v);
    if (distance > vec::epsilon)
        normal = v / distance;
    else
        normal = vec::fvec3(0.0f, 0.0f, 0.0f);
}

float vec::length(const fvec4 &v)
{
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}
vec::fvec4 vec::normal(const fvec4 &v)
{
    float len = length(v);
    if (len > 0.00001f)
    {
        return v / len;
    }
    else
    {
        return fvec4(0.0f, 0.0f, 0.0f, 0.0f);
    }
}
vec::fscalar vec::dot(const fvec4 &a, const fvec4 &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

vec::fvec4 vec::slerp(const fvec4 &a, const fvec4 &b, float t)
{
    float angle = std::acos(vec::dot(a, b));

    if (std::fabs(angle) < 0.0001f)
    {
        return a;
    }
    else
    {

        vec::fscalar inv_denominator = 1.0f / std::sin(angle);
        vec::fscalar factor_a = std::sin((1.0f - t) * angle) * inv_denominator;
        vec::fscalar factor_b = std::sin(t * angle) * inv_denominator;
        return a * factor_a + b * factor_b;
    }
}

vec::fmat4_translation::fmat4_translation(const fvec3 &translation)
    : fmat4(fmat4::column{1, 0, 0, 0},
            fmat4::column{0, 1, 0, 0},
            fmat4::column{0, 0, 1, 0},
            fmat4::column{translation.x, translation.y, translation.z, 1})
{
}

vec::fmat4_rotation::fmat4_rotation(const fvec4 &quaternion)
    : fmat4_rotation(2.0f / vec::dot(quaternion, quaternion), quaternion)
{
}

vec::fmat4_rotation::fmat4_rotation(fscalar s, const fvec4 &rotation)
    : fmat4(
          vec::fmat4::column{
              1 - s * (rotation.y * rotation.y + rotation.z * rotation.z),
              s * (rotation.x * rotation.y + rotation.z * rotation.w),
              s * (rotation.x * rotation.z - rotation.y * rotation.w),
              0,
          },
          vec::fmat4::column{
              s * (rotation.x * rotation.y - rotation.z * rotation.w),
              1 - s * (rotation.x * rotation.x + rotation.z * rotation.z),
              s * (rotation.y * rotation.z + rotation.x * rotation.w),
              0,
          },
          vec::fmat4::column{
              s * (rotation.x * rotation.z + rotation.y * rotation.w),
              s * (rotation.y * rotation.z - rotation.x * rotation.w),
              1 - s * (rotation.x * rotation.x + rotation.y * rotation.y),
              0,
          },
          vec::fmat4::column{
              0,
              0,
              0,
              1,
          })
{
}

vec::fmat4_scale::fmat4_scale(const fvec3 &scale)
    : fmat4(fmat4::column{scale.x, 0, 0, 0},
            fmat4::column{0, scale.y, 0, 0},
            fmat4::column{0, 0, scale.z, 0},
            fmat4::column{0, 0, 0, 1})
{
}

vec::fmat4_transform3::fmat4_transform3(const transform3 &transform)
    : fmat4_transform3(fmat4_translation(transform.translation),
                       fmat4_rotation(transform.rotation),
                       fmat4_scale(transform.scale))
{
}

vec::fmat4_transform3::fmat4_transform3(const fmat4_translation &translation,
                                        const fmat4_rotation &rotation,
                                        const fmat4_scale &scale)
    : fmat4(translation * rotation * scale)
{
}

vec::fmat4_transform3_inverse::fmat4_transform3_inverse(
    const transform3 &transform)
    : fmat4_transform3_inverse(
          fmat4_translation(vec::fvec3(-transform.translation.x,
                                       -transform.translation.y,
                                       -transform.translation.z)),
          fmat4_rotation(vec::fvec4(-transform.rotation.x,
                                    -transform.rotation.y,
                                    -transform.rotation.z,
                                    transform.rotation.w)),
          fmat4_scale(vec::fvec3(1.0f / transform.scale.x,
                                 1.0f / transform.scale.y,
                                 1.0f / transform.scale.z)))
{
}

vec::fmat4_transform3_inverse::fmat4_transform3_inverse(
    const fmat4_translation &translation,
    const fmat4_rotation &rotation,
    const fmat4_scale &scale)
    : fmat4(scale * rotation * translation)
{
}

vec::fmat4 transpose(const vec::fmat4 &m)
{
    vec::fmat4 result;
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            result[col * 4 + row] = m[row * 4 + col];
        }
    }
    return result;
}

vec::fmat3 transpose(const vec::fmat3 &m)
{
    vec::fmat3 result;
    for (int col = 0; col < 3; ++col)
    {
        for (int row = 0; row < 3; ++row)
        {
            result[col * 3 + row] = m[row * 3 + col];
        }
    }
    return result;
}

template <> bool vec::fmat4::operator==(const vec::fmat4 &rhs) const
{
    for (int i = 0; i < 16; i++)
    {
        if (std::fabs(this->indices[i] - rhs.indices[i]) > vec::epsilon)
            return false;
    }
    return true;
}

template <> bool vec::fmat3::operator==(const vec::fmat3 &rhs) const
{
    for (int i = 0; i < 9; i++)
    {
        if (std::fabs(this->indices[i] - rhs.indices[i]) > vec::epsilon)
            return false;
    }
    return true;
}

template <> vec::fmat3::mat3(const vec::fmat4 &m)
{
    indices[0] = m[0];
    indices[1] = m[1];
    indices[2] = m[2];
    indices[3] = m[4];
    indices[4] = m[5];
    indices[5] = m[6];
    indices[6] = m[8];
    indices[7] = m[9];
    indices[8] = m[10];
}

template <>
vec::fmat3 vec::fmat3::operator*(const fmat3 &rhs) const
{
    vec::fmat3 result;
    for (int col = 0; col < 3; ++col)
    {
        for (int row = 0; row < 3; ++row)
        {
            float sum = 0.0f;
            for (int k = 0; k < 3; ++k)
                sum += (*this)[k * 3 + row] * rhs[col * 3 + k];
            result[col * 3 + row] = sum;
        }
    }
    return result;
}