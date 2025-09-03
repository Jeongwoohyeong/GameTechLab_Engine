#pragma once

#include "Vector.h"

struct FMatrix4x4
{
	// 각 방식으로 메모리 접근
	union
	{
		// 2차원 배열
		float m[4][4];
		// 벡터
		FVector4 rows[4];
		// 1차원 배열
		float arr[16];
	};

	FMatrix4x4();

	static FMatrix4x4 CreateIdentity();
	static FMatrix4x4 CreateScale(float x, float y, float z);
	static FMatrix4x4 CreateTranslation(float x, float y, float z);
	static FMatrix4x4 CreateRotationX(float radians);
	static FMatrix4x4 CreateRotationY(float radians);
	static FMatrix4x4 CreateRotationZ(float radians);

	static FMatrix4x4 CreateFromYawPitchRoll(float yaw, float pitch, float roll);

	FMatrix4x4 Transpose();

	FMatrix4x4 operator*(const FMatrix4x4& other) const;

};