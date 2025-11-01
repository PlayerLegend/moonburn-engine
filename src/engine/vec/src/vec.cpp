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

template <> vec::fvec3::decompose::decompose(vec::fvec3 v)
{
    distance = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (distance > 0.00001f)
    {
        normal = v / distance;
    }
    else
    {
        normal.x = 0.0f;
        normal.y = 0.0f;
        normal.z = 0.0f;
    }
}

template <> vec::fvec4 vec::fvec4::operator*(const vec::fvec4 &rhs) const
{
    vec::fvec4 result;
    result.w = this->w * rhs.w - this->x * rhs.x - this->y * rhs.y - this->z * rhs.z;
    result.x = this->w * rhs.x + this->x * rhs.w + this->y * rhs.z - this->z * rhs.y;
    result.y = this->w * rhs.y - this->x * rhs.z + this->y * rhs.w + this->z * rhs.x;
    result.z = this->w * rhs.z + this->x * rhs.y - this->y * rhs.x + this->z * rhs.w;
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
            sum += this->indices[col * 4 + row] * rhs.indices[col];
        result.indices[row] = sum;
    }
    return result;
}