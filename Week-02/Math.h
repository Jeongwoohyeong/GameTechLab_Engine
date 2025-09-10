#pragma once
/*
 * <Coordinate System>
 * - Left-Handed
 * - Y-Up
 *
 * <Vector>
 * - Row vector
 * - Pre-multiplication (v' = vABC)
 *
 * <Matrix>
 * - CPU: Row-major
 * - GPU: Column-major
 */
#include <iostream>
#include <cmath>
#include <cassert>

#ifndef MATH_EPSILON
#define MATH_EPSILON 1e-6f
#endif

namespace Math
{
	inline constexpr float Pi = 3.14159274f;
	inline constexpr float DegToRad = Pi / 180.0f;
	inline constexpr float RadToDeg = 180.0f / Pi;
}

 // ============================================================================
 // FVector (3D)
 // ============================================================================
struct FVector
{
	float X, Y, Z;

	constexpr FVector() noexcept : X(0), Y(0), Z(0) {}
	constexpr FVector(float _X, float _Y, float _Z) noexcept : X(_X), Y(_Y), Z(_Z) {}

	// Named constants
	static constexpr FVector Zero()  noexcept { return FVector(0, 0, 0); }
	static constexpr FVector One()   noexcept { return FVector(1, 1, 1); }
	static constexpr FVector UnitX() noexcept { return FVector(1, 0, 0); }
	static constexpr FVector UnitY() noexcept { return FVector(0, 1, 0); }
	static constexpr FVector UnitZ() noexcept { return FVector(0, 0, 1); }

	// Operators
	constexpr FVector operator+() const noexcept { return *this; }
	constexpr FVector operator-() const noexcept { return FVector(-X, -Y, -Z); }
	constexpr FVector operator*(const FVector& V) const { return FVector(X * V.X, Y * V.Y, Z * V.Z); } // Hadamard product

	constexpr FVector operator+(const FVector& V) const { return FVector(X + V.X, Y + V.Y, Z + V.Z); }
	constexpr FVector operator-(const FVector& V) const { return FVector(X - V.X, Y - V.Y, Z - V.Z); }
	constexpr FVector operator*(float S) const { return FVector(X * S, Y * S, Z * S); }
	constexpr FVector operator/(float S) const { return FVector(X / S, Y / S, Z / S); }

	// NOTE: 왼쪽 피연산자를 수정 후 자기 자신을 반환하니 반환값을 참조형으로 (응용하여 (A += B) += C 같은 체이닝 가능) 
	FVector& operator+=(const FVector& V) { X += V.X; Y += V.Y; Z += V.Z; return *this; }
	FVector& operator-=(const FVector& V) { X -= V.X; Y -= V.Y; Z -= V.Z; return *this; }
	FVector& operator*=(float S) { X *= S; Y *= S, Z *= S; return *this; }
	FVector& operator/=(float S) { X /= S; Y /= S, Z /= S; return *this; }
	
	constexpr bool operator==(const FVector& V) const noexcept { return X == V.X && Y == V.Y && Z == V.Z; }
	constexpr bool operator!=(const FVector& V) const noexcept { return !(*this == V); }

	float Length() const { return sqrt(X * X + Y * Y + Z * Z); }
	float LengthSquared() const { return X * X + Y * Y + Z * Z; }

	bool IsNearlyZero() const noexcept { return (std::abs(X) < MATH_EPSILON) && (std::abs(Y) < MATH_EPSILON) && (std::abs(Z) < MATH_EPSILON); }

	FVector GetNormalized() const
	{
		float Len = Length();
		return (Len > MATH_EPSILON) ? (*this / Len) : FVector::Zero();
	}

	void Normalize()
	{
		float Len = Length();
		if (Len > MATH_EPSILON)
		{
			*this /= Len;
		}
		else
		{
			X = Y = Z = 0.0f;
		}
	}
};

inline FVector operator*(float S, const FVector& V) { return FVector(V.X * S, V.Y * S, V.Z * S); }

inline std::ostream& operator<<(std::ostream& os, const FVector& v)
{
	os << "( " << v.X << ", " << v.Y << ", " << v.Z << " )";
	return os;
}

inline float Distance(const FVector& U, const FVector& V)
{
	return (U - V).Length();
}

inline float Dot(const FVector& U, const FVector& V)
{
	return U.X * V.X + U.Y * V.Y + U.Z * V.Z;
}

inline FVector Cross(const FVector& U, const FVector& V)
{
	return FVector(
		U.Y * V.Z - U.Z * V.Y,
		U.Z * V.X - U.X * V.Z,
		U.X * V.Y - U.Y * V.X
	);
}

inline FVector SwapYZ(const FVector& V)
{
	return FVector(V.X, V.Z, V.Y);
}

struct FVector4
{
	float X, Y, Z, W;

	constexpr FVector4() noexcept : X(0), Y(0), Z(0), W(0) {}
	constexpr FVector4(float _X, float _Y, float _Z, float _W) noexcept : X(_X), Y(_Y), Z(_Z), W(_W) {}
	constexpr explicit FVector4(const FVector& V, float _W) noexcept : X(V.X), Y(V.Y), Z(V.Z), W(_W) {} // FVector 넣었다가 암시적 변환 되어 버리는 것 방지

	static constexpr FVector4 MakePoint(const FVector& P) noexcept { return FVector4(P, 1.0f); }
	static constexpr FVector4 MakeVector(const FVector& V) noexcept { return FVector4(V, 0.0f); }

	bool IsPoint() const noexcept { return std::abs(W - 1.0f) < MATH_EPSILON; }
	bool IsVector() const noexcept { return std::abs(W) < MATH_EPSILON; }
};

struct FQuaternion {
	float X = 0, Y = 0, Z = 0, W = 1;

	// 두 쿼터니언을 곱한다
	static FQuaternion Multiply(const FQuaternion& q1, const FQuaternion& q2) {
		FQuaternion result;
		result.W = q1.W * q2.W - q1.X * q2.X - q1.Y * q2.Y - q1.Z * q2.Z;
		result.X = q1.W * q2.X + q1.X * q2.W + q1.Y * q2.Z - q1.Z * q2.Y;
		result.Y = q1.W * q2.Y - q1.X * q2.Z + q1.Y * q2.W + q1.Z * q2.X;
		result.Z = q1.W * q2.Z + q1.X * q2.Y - q1.Y * q2.X + q1.Z * q2.W;
		return result;
	}

	// 오일러 각도(Z -> X -> Y 순서)로부터 쿼터니언을 생성
	static FQuaternion CreateFromEulerAngles(FVector Rotation) {
		float halfZ = Rotation.Z * 0.5f;
		float halfX = Rotation.X * 0.5f;
		float halfY = Rotation.Y * 0.5f;

		FQuaternion qZ = { 0, 0, sin(halfZ), cos(halfZ) };
		FQuaternion qX = { sin(halfX), 0, 0, cos(halfX) };
		FQuaternion qY = { 0, sin(halfY), 0, cos(halfY) };

		return Multiply(Multiply(qZ, qX), qY);
	}

	// 쿼터니언을 Z -> X -> Y 순서의 오일러 각도로 변환
	static FVector ToEulerAngles(FQuaternion Q) {
		FVector Angles;

		// 1. Pitch (X축 회전) 계산 및 짐벌 락 확인
		// sin(pitch) 값을 계산합니다. 이 값이 1 또는 -1에 가까워지면 짐벌 락 상태입니다.
		float sinp = 2.0f * (Q.W * Q.X - Q.Y * Q.Z);

		// 2. 짐벌 락 상태 처리 (Pitch가 +/- 90도에 가까운 경우)
		// 부동소수점 오차를 고려해 1.0f보다 약간 작은 값과 비교합니다.
		if (std::abs(sinp) >= 0.99999f) {
			// Pitch(X)를 90도 또는 -90도로 고정합니다.
			Angles.X = std::copysign(Math::Pi / 2.0f, sinp);

			// 짐벌 락 상태에서는 Roll(Y축)을 0으로 고정하고,
			// 모든 회전을 Yaw(Z축)에 계산합니다.
			Angles.Z = 2.0f * std::atan2(Q.Z, Q.W);
			Angles.Y = 0.0f;
		}
		// 3. 일반적인 경우 (짐벌 락이 아닐 때)
		else {
			// Pitch (X축 회전)
			Angles.X = std::asin(sinp);

			// Yaw (Z축 회전)
			float siny_cosp = 2.0f * (Q.W * Q.Z + Q.X * Q.Y);
			float cosy_cosp = 1.0f - 2.0f * (Q.X * Q.X + Q.Z * Q.Z);
			Angles.Z = std::atan2(siny_cosp, cosy_cosp);

			// Roll (Y축 회전)
			float sinr_cosp = 2.0f * (Q.W * Q.Y + Q.X * Q.Z);
			float cosr_cosp = 1.0f - 2.0f * (Q.X * Q.X + Q.Y * Q.Y);
			Angles.Y = std::atan2(sinr_cosp, cosr_cosp);
		}

		return Angles;
	}

	static FQuaternion CreateFromAxisAngle(const FVector& axis, float angle)
	{
		// 1. 각도의 절반과 그 sin 값을 계산합니다.
		float halfAngle = angle * 0.5f;
		float s = std::sin(halfAngle);

		FQuaternion result;

		// 2. 공식에 따라 쿼터니언의 각 요소를 계산합니다.
		result.W = std::cos(halfAngle); // w 요소는 코사인
		result.X = axis.X * s;         // x, y, z 요소는 축 벡터에 사인값을 곱함
		result.Y = axis.Y * s;
		result.Z = axis.Z * s;

		return result;
	}

	float Magnitude() const {
		return sqrt(X * X + Y * Y + Z * Z + W * W);
	}

	// 쿼터니언을 정규화 (크기를 1로 만듦)
	void Normalize() {
		float mag = Magnitude();

		// 크기가 0에 매우 가까운 경우 나누기 오류를 방지
		// (0으로 나누면 안 되므로)
		if (mag > 0.00001f) {
			X /= mag;
			Y /= mag;
			Z /= mag;
			W /= mag;
		}
		else 
		{
			// 크기가 0인 쿼터니언은 유효하지 않으므로,
			// 안전하게 기본 회전값(Identity)으로 리셋
			X = 0; Y = 0; Z = 0; W = 1;
		}
	}
};

struct FMatrix
{
	float M[4][4];

	static constexpr FMatrix Identity() noexcept
	{
		return FMatrix{ {
			{1,0,0,0},
			{0,1,0,0},
			{0,0,1,0},
			{0,0,0,1}
		} };
	}

	float* operator[](int Row) { return M[Row]; }

	FMatrix operator*(const FMatrix& N) const
	{
		FMatrix Result = {};
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				Result.M[i][j] = M[i][0] * N.M[0][j] + M[i][1] * N.M[1][j] + M[i][2] * N.M[2][j] + M[i][3] * N.M[3][j];
			}
		}
		return Result;
	}

	static FMatrix MakeScale(const FVector& S)
	{
		return FMatrix{ {
			{ S.X,    0,   0,   0 },
			{   0,  S.Y,   0,   0 },
			{   0,    0, S.Z,   0 },
			{   0,    0,   0,   1 }
		} };
	}

	static FMatrix MakeRotationX(float Rad)
	{
		float C = cos(Rad);
		float S = sin(Rad);
		return FMatrix{ {
			{  1,  0,  0,  0 },
			{  0,  C,  S,  0 },
			{  0, -S,  C,  0 },
			{  0,  0,  0,  1 }
		} };
	}

	static FMatrix MakeRotationY(float Rad)
	{
		FMatrix Result = Identity();
		float C = cos(Rad);
		float S = sin(Rad);
		return FMatrix{ {
			{  C,  0, -S,  0 },
			{  0,  1,  0,  0 },
			{  S,  0,  C,  0 },
			{  0,  0,  0,  1 }
		} };
	}

	static FMatrix MakeRotationZ(float Rad)
	{
		FMatrix Result = Identity();
		float C = cos(Rad);
		float S = sin(Rad);
		return FMatrix{ {
			{  C,  S,  0,  0 },
			{ -S,  C,  0,  0 },
			{  0,  0,  1,  0 },
			{  0,  0,  0,  1 }
		} };
	}

	static FMatrix MakeRotation(const FVector& RadVec) // R: in Radian
	{
		// (Z -> X -> Y) 순서 바꾸는 거 위험!
		return MakeRotationZ(RadVec.Z) * MakeRotationX(RadVec.X) * MakeRotationY(RadVec.Y);
	}

	static FVector GetRotationFromMatrix(const FMatrix& Mat)
	{
		FVector Result;
		float SinY = Mat.M[2][0];
		Result.Y = std::asin(SinY);
		float CosY = std::sqrt(1.0f - SinY * SinY);
		if(CosY > MATH_EPSILON)
		{
			Result.Z = std::atan2(-Mat.M[1][0], Mat.M[0][0]);
			Result.X = std::atan2(-Mat.M[2][1], Mat.M[2][2]);
		}
		else
		{
			// 짐벌락 발생
			Result.X = 0.0f; // 편의상 0으로 설정
			if (SinY > 0) // +90도
			{
				Result.Z = std::atan2(Mat.M[0][1], Mat.M[1][1]);
			}
			else // -90도
			{
				Result.Z = std::atan2(-Mat.M[0][1], Mat.M[1][1]);
			}
		}
		
		return Result;
	}


	static FMatrix MakeTranslation(const FVector& T)
	{
		return FMatrix{ {
			{   1,    0,   0,   0 },
			{   0,    1,   0,   0 },
			{   0,    0,   1,   0 },
			{ T.X,  T.Y, T.Z,   1 }
		} };
	}

	static FMatrix Transpose(const FMatrix& Mat)
	{
		FMatrix Result = {};
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				Result.M[i][j] = Mat.M[j][i];
			}
		}
		return Result;
	}

	/* 왜 필요한지 모르겠음, 아직 검증되지 않았음 */
	//static FMatrix MakeView(const FVector& Location, const FVector& Rotation, const FVector& Target, const FVector& Up)
	//{
	//	FVector Forward = (Target - Location); // Z축
	//	Forward.Normalize();
	//	FVector Right = Cross(Up, Forward); // X축
	//	Right.Normalize();
	//	FVector TrueUp = Cross(Forward, Right); // Y축

	//	// 카메라의 기저 벡터를 행렬로 표현
	//	FMatrix Basis = Identity();
	//	Basis.M[0][0] = Right.X;
	//	Basis.M[0][1] = Right.Y;
	//	Basis.M[0][2] = Right.Z;
	//	Basis.M[1][0] = TrueUp.X;
	//	Basis.M[1][1] = TrueUp.Y;
	//	Basis.M[1][2] = TrueUp.Z;
	//	Basis.M[2][0] = Forward.X;
	//	Basis.M[2][1] = Forward.Y;
	//	Basis.M[2][2] = Forward.Z;

	//	// 카메라 기적 벡터를 회전
	//	Basis = Basis * MakeRotation(Rotation);
	//	Right = { Basis.M[0][0], Basis.M[0][1], Basis.M[0][2] };
	//	TrueUp = { Basis.M[1][0], Basis.M[1][1], Basis.M[1][2] };
	//	Forward = { Basis.M[2][0], Basis.M[2][1], Basis.M[2][2] };

	//	FMatrix Result = Identity();

	//	// 카메라 기저 벡터와 위치로 뷰 행렬 구성
	//	Result.M[0][0] = Right.X;
	//	Result.M[0][1] = Right.Y;
	//	Result.M[0][2] = Right.Z;
	//	Result.M[0][3] = 0.0f;

	//	Result.M[1][0] = TrueUp.X;
	//	Result.M[1][1] = TrueUp.Y;
	//	Result.M[1][2] = TrueUp.Z;
	//	Result.M[1][3] = 0.0f;

	//	Result.M[2][0] = Forward.X;
	//	Result.M[2][1] = Forward.Y;
	//	Result.M[2][2] = Forward.Z;
	//	Result.M[2][3] = 0.0f;

	//	Result.M[3][0] = -Dot(Right ,Location);
	//	Result.M[3][1] = -Dot(TrueUp, Location);
	//	Result.M[3][2] = -Dot(Forward, Location);
	//	Result.M[3][3] = 1.0f;

	//	return Result;
	//}

	static FMatrix MakeView(const FVector& Location, const FVector& Rotation)
	{
		FMatrix InverseRotation = Transpose(MakeRotation(Rotation)); // R^-1 = R^T
		FMatrix InverseTranslation = MakeTranslation(-Location); // T^-1 = -T

		return InverseTranslation * InverseRotation; // T^-1 * R^-1
	}

	/* 원인은 모르지만 동작하지 않는다. 추후 리뷰  */
	//static FMatrix MakePerspectiveFovLH(float fovY, float aspect, float zn, float zf)
	//{
	//	// 주의: Direct3D NDC z ∈ [0,1]
	//	const float yScale = 1.0f / std::tan(fovY * 0.5f);
	//	const float xScale = yScale / aspect;
	//	const float Q = zf / (zf - zn);          // LH

	//	FMatrix m = FMatrix::Identity();
	//	m.M[0][0] = xScale;
	//	m.M[1][1] = yScale;
	//	m.M[2][2] = Q;
	//	m.M[2][3] = (-zn * zf) / (zf - zn);      // row-major에서는 (2,3)
	//	m.M[3][2] = 1.0f;                        // row-major에서는 (3,2)
	//	m.M[3][3] = 0.0f;
	//	return m;
	//}

	static FMatrix MakePerspective(float FovY, float Aspect, float NearZ, float FarZ)
	{
		assert(Aspect > 0 && NearZ > 0 && FarZ > NearZ);
		assert(FovY > MATH_EPSILON && FovY < Math::Pi );

		float Ys = 1.0f / std::tan(FovY * 0.5f);
		float Xs = Ys / Aspect;

		float A = FarZ / (FarZ - NearZ);
		float B = -NearZ * FarZ / (FarZ - NearZ);

		FMatrix Result = {};
		Result.M[0][0] = Xs;
		Result.M[1][1] = Ys;
		Result.M[2][2] = A;
		Result.M[2][3] = 1.0f;   // w' = z
		Result.M[3][2] = B;      // z' = z*A + B
		Result.M[3][3] = 0.0f;

		return Result;
	}

	static FMatrix MakeOrthographic(float left, float right, float bottom, float top, float nearZ, float farZ)
	{
		// x축 범위 NDC [-1, 1] 정규화
		float m00 = (2.0f / (right - left));
		// y축 범위 NDC [-1, 1] 정규화
		float m11 = (2.0f / (top - bottom));
		// z축 범위 NDC [0, 1] 정규화
		float m22 = (1.0f / (farZ - nearZ));
		// 뷰 볼륨 중심을 원점 (0, 0, 0)으로 이동
		float m03 = -(right + left) / (right - left);
		float m13 = -(top + bottom) / (top - bottom);		
		float m23 = -nearZ / (farZ - nearZ);

		FMatrix Result{ {
			{	m00,   0.0f,  0.0f,  m03 },
			{  0.0f,	m11,  0.0f,  m13 },
			{  0.0f,   0.0f,   m22,  m23 },
			{  0.0f,   0.0f,  0.0f,  1.0f }
		} };

		return Result;
	}

	// 쿼터니언으로부터 왼손 좌표계 기준 회전 행렬 생성
	static FMatrix MakeRotationFromQuaternion(const FQuaternion& Q)
	{
		FMatrix Result = {};
		float XX = Q.X * Q.X;
		float YY = Q.Y * Q.Y;
		float ZZ = Q.Z * Q.Z;
		float XY = Q.X * Q.Y;
		float XZ = Q.X * Q.Z;
		float YZ = Q.Y * Q.Z;
		float WX = Q.W * Q.X;
		float WY = Q.W * Q.Y;
		float WZ = Q.W * Q.Z;

		Result.M[0][0] = 1.0f - 2.0f * (YY + ZZ);
		Result.M[0][1] = 2.0f * (XY + WZ);
		Result.M[0][2] = 2.0f * (XZ - WY);
		Result.M[0][3] = 0.0f;

		Result.M[1][0] = 2.0f * (XY - WZ);
		Result.M[1][1] = 1.0f - 2.0f * (XX + ZZ);
		Result.M[1][2] = 2.0f * (YZ + WX);
		Result.M[1][3] = 0.0f;

		Result.M[2][0] = 2.0f * (XZ + WY);
		Result.M[2][1] = 2.0f * (YZ - WX);
		Result.M[2][2] = 1.0f - 2.0f * (XX + YY);
		Result.M[2][3] = 0.0f;

		Result.M[3][0] = 0.0f;
		Result.M[3][1] = 0.0f;
		Result.M[3][2] = 0.0f;
		Result.M[3][3] = 1.0f;

		return Result;
	}

	// 여기에 get x axis, y axis, z axis 넣어줘. 우리 왼손 좌표계임
	FVector GetAxisX() { return FVector(M[0][0], M[1][0], M[2][0]); }
	FVector GetAxisY() { return FVector(M[0][1], M[1][1], M[2][1]); }
	FVector GetAxisZ() { return FVector(M[0][2], M[1][2], M[2][2]); }
};

inline float DegToRad(float Degree)
{
	return Degree * Math::DegToRad;
}

inline FVector DegToRad(const FVector& Degree)
{
	return Degree * Math::DegToRad;
}

inline float RadToDeg(float Radian)
{
	return Radian * Math::RadToDeg;
}

inline FVector RadToDeg(const FVector& Radian)
{
	return Radian * Math::RadToDeg;

}

inline FVector ToFVector3(const FVector4& V4)
{
    return FVector(V4.X, V4.Y, V4.Z);
}

inline FVector4 operator*(const FVector4& V, const FMatrix& M)
{
	FVector4 Result;
	Result.X = V.X * M.M[0][0] + V.Y * M.M[1][0] + V.Z * M.M[2][0] + V.W * M.M[3][0];
	Result.Y = V.X * M.M[0][1] + V.Y * M.M[1][1] + V.Z * M.M[2][1] + V.W * M.M[3][1];
	Result.Z = V.X * M.M[0][2] + V.Y * M.M[1][2] + V.Z * M.M[2][2] + V.W * M.M[3][2];
	Result.W = V.X * M.M[0][3] + V.Y * M.M[1][3] + V.Z * M.M[2][3] + V.W * M.M[3][3];

	return Result;
}

inline FVector MultiplyVecMat(const FVector& v, const FMatrix& M)
{
	const float x =v.X, y = v.Y, z = v.Z;
	const float X = x * M.M[0][0] + y * M.M[1][0] + z * M.M[2][0] + 1.0f * M.M[3][0];
	const float Y = x * M.M[0][1] + y * M.M[1][1] + z * M.M[2][1] + 1.0f * M.M[3][1];
	const float Z = x * M.M[0][2] + y * M.M[1][2] + z * M.M[2][2] + 1.0f * M.M[3][2];
	const float W = x * M.M[0][3] + y * M.M[1][3] + z * M.M[2][3] + 1.0f * M.M[3][3];
	// 일반 뷰/월드 행렬은 W=1이므로 보통 나눌 필요 없음. (투영행렬은 필요)
	return FVector(X, Y, Z);
}
//struct FQuaternion
//{
//	float X, Y, Z, W; // W + Xi + Yj + Zk
//	constexpr FQuaternion() noexcept : X(0), Y(0), Z(0), W(1) {}
//	constexpr FQuaternion(float _X, float _Y, float _Z, float _W) noexcept : X(_X), Y(_Y), Z(_Z), W(_W) {}
//
//	static FQuaternion FromAxisAngle(const FVector& Axis, float AngleRad)
//	{
//		float HalfAngle = AngleRad * 0.5f;
//		float S = sin(HalfAngle);
//		return FQuaternion(Axis.X * S, Axis.Y * S, Axis.Z * S, cos(HalfAngle));
//	}
//	static FQuaternion FromEuler(const FVector& EulerRad)
//	{
//		float Cy = cos(EulerRad.Z * 0.5f);
//		float Sy = sin(EulerRad.Z * 0.5f);
//		float Cp = cos(EulerRad.Y * 0.5f);
//		float Sp = sin(EulerRad.Y * 0.5f);
//		float Cr = cos(EulerRad.X * 0.5f);
//		float Sr = sin(EulerRad.X * 0.5f);
//		FQuaternion Q;
//		Q.W = Cr * Cp * Cy + Sr * Sp * Sy;
//		Q.X = Sr * Cp * Cy - Cr * Sp * Sy;
//		Q.Y = Cr * Sp * Cy + Sr * Cp * Sy;
//		Q.Z = Cr * Cp * Sy - Sr * Sp * Cy;
//		return Q;
//	}
//
//	constexpr FQuaternion operator*(FQuaternion Other) const noexcept
//	{
//		return FQuaternion(
//			W * Other.X + X * Other.W + Y * Other.Z - Z * Other.Y,
//			W * Other.Y - X * Other.Z + Y * Other.W + Z * Other.X,
//			W * Other.Z + X * Other.Y - Y * Other.X + Z * Other.W,
//			W * Other.W - X * Other.X - Y * Other.Y - Z * Other.Z
//		);
//	}
//
//	// 켤레 쿼터니언을 구합니다.
//	constexpr FQuaternion GetConjugate() const noexcept
//	{
//		return FQuaternion(-X, -Y, -Z, W);
//	}
//
//	inline FMatrix MakeRotationQuaternion() const
//	{
//		// https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation#Quaternion-derived_rotation_matrix
//		// 위키백과의 notation은 colum vector & column-major 기준이므로 전치해서 사용
//		FMatrix Result = FMatrix::Identity();
//		float XX = X * X;
//		float YY = Y * Y;
//		float ZZ = Z * Z;
//		float XY = X * Y;
//		float XZ = X * Z;
//		float YZ = Y * Z;
//		float WX = W * X;
//		float WY = W * Y;
//		float WZ = W * Z;
//		Result.M[0][0] = 1.0f - 2.0f * (YY + ZZ);
//		Result.M[0][1] = 2.0f * (XY + WZ);
//		Result.M[0][2] = 2.0f * (XZ - WY);
//		Result.M[1][0] = 2.0f * (XY - WZ);
//		Result.M[1][1] = 1.0f - 2.0f * (XX + ZZ);
//		Result.M[1][2] = 2.0f * (YZ + WX);
//		Result.M[2][0] = 2.0f * (XZ + WY);
//		Result.M[2][1] = 2.0f * (YZ - WX);
//		Result.M[2][2] = 1.0f - 2.0f * (XX + YY);
//		return Result;
//	}
//};
