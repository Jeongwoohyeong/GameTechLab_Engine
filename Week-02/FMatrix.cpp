#include <cmath>
#include "FMatrix.h"

FMatrix::FMatrix()
{
    rows[0] = { 1.0f, 0.0f, 0.0f, 0.0f };
    rows[1] = { 0.0f, 1.0f, 0.0f, 0.0f };
    rows[2] = { 0.0f, 0.0f, 1.0f, 0.0f };
    rows[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
}

FMatrix FMatrix::CreateIdentity()
{
    // 기본 생성자가 생성하는 단위행렬 반환
    return FMatrix();
}

FMatrix FMatrix::CreateScale(float x, float y, float z)
{
    FMatrix Scale;

    Scale.m[0][0] = x;
    Scale.m[1][1] = y;
    Scale.m[2][2] = z;

    return Scale;
}

FMatrix FMatrix::CreateTranslation(float x, float y, float z)
{
    FMatrix translation;
    translation.m[3][0] = x;
    translation.m[3][1] = y;
    translation.m[3][2] = z;

    return translation;
}

FMatrix FMatrix::CreateRotationX(float radians)
{
    FMatrix rotationX;
    float cosTheta = cos(radians);
    float sinTheta = sin(radians);

    rotationX.m[1][1] = cosTheta;
    rotationX.m[1][2] = -sinTheta;
    rotationX.m[2][1] = sinTheta;
    rotationX.m[2][2] = cosTheta;

    return rotationX;
}

FMatrix FMatrix::CreateRotationY(float radians)
{
    FMatrix rotationX;
    float cosTheta = cos(radians);
    float sinTheta = sin(radians);

    rotationX.m[0][0] = cosTheta;
    rotationX.m[0][2] = sinTheta;
    rotationX.m[2][0] = -sinTheta;
    rotationX.m[2][2] = cosTheta;

    return rotationX;
}

FMatrix FMatrix::CreateRotationZ(float radians)
{
    FMatrix rotationX;
    float cosTheta = cos(radians);
    float sinTheta = sin(radians);

    rotationX.m[0][0] = cosTheta;
    rotationX.m[0][1] = -sinTheta;
    rotationX.m[1][0] = sinTheta;
    rotationX.m[1][1] = cosTheta;

    return rotationX;
}

FMatrix FMatrix::CreateFromYawPitchRoll(float yaw, float pitch, float roll)
{
    return CreateRotationZ(roll) * CreateRotationX(pitch) * CreateRotationY(yaw);
}

FMatrix FMatrix::Transpose()
{
    FMatrix transpose;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4;j++)
        {
            transpose.m[i][j] = m[j][i];
        }
    }

    return transpose;
}

FMatrix FMatrix::operator*(const FMatrix& other) const
{
    FMatrix result;

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