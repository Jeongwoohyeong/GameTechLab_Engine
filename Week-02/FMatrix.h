#pragma once
#include "FVector.h"

static constexpr float PI = 3.1415926f;

struct FMatrix
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

	FMatrix();

	static FMatrix CreateIdentity();
	static FMatrix CreateScale(float x, float y, float z);
	static FMatrix CreateScale(const FVector&);

	static FMatrix CreateTranslation(float x, float y, float z);
	static FMatrix CreateTranslation(const FVector&);

	static FMatrix CreateRotationX(float degree);
	static FMatrix CreateRotationY(float degree);
	static FMatrix CreateRotationZ(float degree);

	static FMatrix CreateFromYawPitchRoll(float yaw, float pitch, float roll);

	FMatrix Transpose();

	FMatrix operator*(const FMatrix& other) const;

};
