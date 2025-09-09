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
		// Yaw -> Pitch -> Roll (Z -> X -> Y) 순서 바꾸는 거 위험!
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

struct FQuaternion
{
	float X, Y, Z, W; // W + Xi + Yj + Zk
	constexpr FQuaternion() noexcept : X(0), Y(0), Z(0), W(1) {}
	constexpr FQuaternion(float _X, float _Y, float _Z, float _W) noexcept : X(_X), Y(_Y), Z(_Z), W(_W) {}

	static FQuaternion FromAxisAngle(const FVector& Axis, float AngleRad)
	{
		float HalfAngle = AngleRad * 0.5f;
		float S = sin(HalfAngle);
		return FQuaternion(Axis.X * S, Axis.Y * S, Axis.Z * S, cos(HalfAngle));
	}
	static FQuaternion FromEuler(const FVector& EulerRad)
	{
		float Cy = cos(EulerRad.Z * 0.5f);
		float Sy = sin(EulerRad.Z * 0.5f);
		float Cp = cos(EulerRad.Y * 0.5f);
		float Sp = sin(EulerRad.Y * 0.5f);
		float Cr = cos(EulerRad.X * 0.5f);
		float Sr = sin(EulerRad.X * 0.5f);
		FQuaternion Q;
		Q.W = Cr * Cp * Cy + Sr * Sp * Sy;
		Q.X = Sr * Cp * Cy - Cr * Sp * Sy;
		Q.Y = Cr * Sp * Cy + Sr * Cp * Sy;
		Q.Z = Cr * Cp * Sy - Sr * Sp * Cy;
		return Q;
	}

	constexpr FQuaternion operator*(FQuaternion Other) const noexcept
	{
		return FQuaternion(
			W * Other.X + X * Other.W + Y * Other.Z - Z * Other.Y,
			W * Other.Y - X * Other.Z + Y * Other.W + Z * Other.X,
			W * Other.Z + X * Other.Y - Y * Other.X + Z * Other.W,
			W * Other.W - X * Other.X - Y * Other.Y - Z * Other.Z
		);
	}

	// 켤레 쿼터니언을 구합니다.
	constexpr FQuaternion GetConjugate() const noexcept
	{
		return FQuaternion(-X, -Y, -Z, W);
	}

	inline FMatrix MakeRotationQuaternion() const
	{
		// https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation#Quaternion-derived_rotation_matrix
		// 위키백과의 notation은 colum vector & column-major 기준이므로 전치해서 사용
		FMatrix Result = FMatrix::Identity();
		float XX = X * X;
		float YY = Y * Y;
		float ZZ = Z * Z;
		float XY = X * Y;
		float XZ = X * Z;
		float YZ = Y * Z;
		float WX = W * X;
		float WY = W * Y;
		float WZ = W * Z;
		Result.M[0][0] = 1.0f - 2.0f * (YY + ZZ);
		Result.M[0][1] = 2.0f * (XY + WZ);
		Result.M[0][2] = 2.0f * (XZ - WY);
		Result.M[1][0] = 2.0f * (XY - WZ);
		Result.M[1][1] = 1.0f - 2.0f * (XX + ZZ);
		Result.M[1][2] = 2.0f * (YZ + WX);
		Result.M[2][0] = 2.0f * (XZ + WY);
		Result.M[2][1] = 2.0f * (YZ - WX);
		Result.M[2][2] = 1.0f - 2.0f * (XX + YY);
		return Result;
	}
};