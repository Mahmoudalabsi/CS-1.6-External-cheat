#pragma once
#include <corecrt_math.h>
static class Vector3
{
public:
        float x, y, z;

        Vector3()
        {
                x = y = z = 0.0f;
        }

        Vector3(float X, float Y, float Z)
        {
                x = X; y = Y; z = Z;
        }

        Vector3(float XYZ)
        {
                x = XYZ; y = XYZ; z = XYZ;
        }

        Vector3(float* v)
        {
                x = v[0]; y = v[1]; z = v[2];
        }

        Vector3(const float* v)
        {
                x = v[0]; y = v[1]; z = v[2];
        }

        inline Vector3& operator=(const Vector3& v)
        {
                x = v.x; y = v.y; z = v.z; return *this;
        }

        inline Vector3& operator=(const float* v)
        {
                x = v[0]; y = v[1]; z = v[2]; return *this;
        }

        inline float& operator[](int i)
        {
                return ((float*)this)[i];
        }

        inline float operator[](int i) const
        {
                return ((float*)this)[i];
        }

        inline Vector3& operator+=(const Vector3& v)
        {
                x += v.x; y += v.y; z += v.z; return *this;
        }

        inline Vector3& operator-=(const Vector3& v)
        {
                x -= v.x; y -= v.y; z -= v.z; return *this;
        }

        inline Vector3& operator*=(const Vector3& v)
        {
                x *= v.x; y *= v.y; z *= v.z; return *this;
        }

        inline Vector3& operator/=(const Vector3& v)
        {
                x /= v.x; y /= v.y; z /= v.z; return *this;
        }

        inline Vector3& operator+=(float v)
        {
                x += v; y += v; z += v; return *this;
        }

        inline Vector3& operator-=(float v)
        {
                x -= v; y -= v; z -= v; return *this;
        }

        inline Vector3& operator*=(float v)
        {
                x *= v; y *= v; z *= v; return *this;
        }

        inline Vector3& operator/=(float v)
        {
                x /= v; y /= v; z /= v; return *this;
        }

        inline Vector3 operator-() const
        {
                return Vector3(-x, -y, -z);
        }

        inline Vector3 operator+(const Vector3& v) const
        {
                return Vector3(x + v.x, y + v.y, z + v.z);
        }

        inline Vector3 operator-(const Vector3& v) const
        {
                return Vector3(x - v.x, y - v.y, z - v.z);
        }

        inline Vector3 operator*(const Vector3& v) const
        {
                return Vector3(x * v.x, y * v.y, z * v.z);
        }

        inline Vector3 operator/(const Vector3& v) const
        {
                return Vector3(x / v.x, y / v.y, z / v.z);
        }

        inline Vector3 operator+(float v) const
        {
                return Vector3(x + v, y + v, z + v);
        }

        inline Vector3 operator-(float v) const
        {
                return Vector3(x - v, y - v, z - v);
        }

        inline Vector3 operator*(float v) const
        {
                return Vector3(x * v, y * v, z * v);
        }

        inline Vector3 operator/(float v) const
        {
                return Vector3(x / v, y / v, z / v);
        }

        inline float Length() const
        {
                return sqrtf(x * x + y * y + z * z);
        }

        inline float LengthSqr() const
        {
                return (x * x + y * y + z * z);
        }

        inline float LengthXY() const
        {
                return sqrtf(x * x + y * y);
        }

        inline float LengthXZ() const
        {
                return sqrtf(x * x + z * z);
        }

        inline float DistTo(const Vector3& v) const
        {
                return (*this - v).Length();
        }

        inline float Dot(const Vector3& v) const
        {
                return (x * v.x + y * v.y + z * v.z);
        }

        inline Vector3 Cross(const Vector3& v) const
        {
                return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
        }

        inline bool IsZero() const
        {
                return (x > -0.01f && x < 0.01f
                        && y > -0.01f && y < 0.01f
                        && z > -0.01f && z < 0.01f);
        }
};

// 4x4 column-major view matrix at hw.dll+0xEC9780
// Memory layout (column-major, 16 floats = 64 bytes):
//   Col0 [offset  0]: m[0]=Rx,  m[1]=Ux,  m[2]=Fx,  m[3]=0
//   Col1 [offset 16]: m[4]=Ry,  m[5]=Uy,  m[6]=Fy,  m[7]=0
//   Col2 [offset 32]: m[8]=Rz,  m[9]=Uz,  m[10]=Fz, m[11]=0
//   Col3 [offset 48]: m[12]=-R·eye, m[13]=-U·eye, m[14]=-F·eye, m[15]=1
// Where R=Right, U=Up, F=Forward, eye=camera position
//
// To extract vectors:
//   Right   = { m[0], m[4], m[8]  }
//   Up      = { m[1], m[5], m[9]  }
//   Forward = { m[2], m[6], m[10] }
//   Origin  = { -(m[12]*m[0]+m[13]*m[1]+m[14]*m[2]),
//               -(m[12]*m[4]+m[13]*m[5]+m[14]*m[6]),
//               -(m[12]*m[8]+m[13]*m[9]+m[14]*m[10]) }
struct ViewMatrix
{
        float m[16];

        Vector3 GetRight()   const { return Vector3(m[0], m[4], m[8]);  }
        Vector3 GetUp()      const { return Vector3(m[1], m[5], m[9]);  }
        Vector3 GetForward() const { return Vector3(m[2], m[6], m[10]); }
        Vector3 GetOrigin()  const
        {
                return Vector3(
                        -(m[12] * m[0] + m[13] * m[1] + m[14] * m[2]),
                        -(m[12] * m[4] + m[13] * m[5] + m[14] * m[6]),
                        -(m[12] * m[8] + m[13] * m[9] + m[14] * m[10])
                );
        }
};