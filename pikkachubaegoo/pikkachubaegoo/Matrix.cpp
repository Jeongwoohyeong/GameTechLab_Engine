#include <cmath>
#include "Matrix.h"

FMatrix4x4::FMatrix4x4()
{
    rows[0] = { 1.0f, 0.0f, 0.0f, 0.0f };
    rows[1] = { 0.0f, 1.0f, 0.0f, 0.0f };
    rows[2] = { 0.0f, 0.0f, 1.0f, 0.0f };
    rows[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
}

FMatrix4x4 FMatrix4x4::CreateIdentity()
{
    // 기본 생성자가 생성하는 단위행렬 반환
    return FMatrix4x4();
}

FMatrix4x4 FMatrix4x4::CreateScale(float x, float y, float z)
{
    FMatrix4x4 scale;

    scale.m[0][0] = x;
    scale.m[1][1] = y;
    scale.m[2][2] = z;

    return scale;
}

FMatrix4x4 FMatrix4x4::CreateTranslation(float x, float y, float z)
{
    FMatrix4x4 translation;
    translation.m[3][0] = x;
    translation.m[3][1] = y;
    translation.m[3][2] = z;

    return translation;
}

FMatrix4x4 FMatrix4x4::CreateRotationX(float radians)
{
    FMatrix4x4 rotationX;
    float cosTheta = cos(radians);
    float sinTheta = sin(radians);

    rotationX.m[1][1] = cosTheta;
    rotationX.m[1][2] = -sinTheta;
    rotationX.m[2][1] = sinTheta;
    rotationX.m[2][2] = cosTheta;

    return rotationX;
}

FMatrix4x4 FMatrix4x4::CreateRotationY(float radians)
{
    FMatrix4x4 rotationX;
    float cosTheta = cos(radians);
    float sinTheta = sin(radians);

    rotationX.m[0][0] = cosTheta;
    rotationX.m[0][2] = sinTheta;
    rotationX.m[2][0] = -sinTheta;
    rotationX.m[2][2] = cosTheta;

    return rotationX;
}

FMatrix4x4 FMatrix4x4::CreateRotationZ(float radians)
{
    FMatrix4x4 rotationX;
    float cosTheta = cos(radians);
    float sinTheta = sin(radians);

    rotationX.m[0][0] = cosTheta;
    rotationX.m[0][1] = -sinTheta;
    rotationX.m[1][0] = sinTheta;
    rotationX.m[1][1] = cosTheta;

    return rotationX;
}

FMatrix4x4 FMatrix4x4::CreateFromYawPitchRoll(float yaw, float pitch, float roll)
{    
    return CreateRotationZ(roll) * CreateRotationX(pitch) * CreateRotationY(yaw);
}

FMatrix4x4 FMatrix4x4::Transpose()
{
    FMatrix4x4 transpose;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4;j++)
        {
            transpose.m[i][j] = m[j][i];
        }
    }

    return transpose;
}

FMatrix4x4 FMatrix4x4::operator*(const FMatrix4x4& other) const
{
    FMatrix4x4 result;
    
    for (int i = 0; i < 4;i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.m[i][j] = 
                ((m[i][0] * other.m[0][j]) + 
                (m[i][1] * other.m[1][j]) + 
                (m[i][2] * other.m[2][j]) +
                (m[i][3] * other.m[3][j]));
        }
    }


    return result;
}
