#pragma once
#include <cmath>

struct FVector
{
	float x, y, z;

	FVector(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) :x(_x), y(_y), z(_z) {};

	// 복사 생성자, 복사 대입 연산자는 컴파일러 생성에 맡김

	FVector operator+(const FVector& other) const
	{
		return FVector((this->x + other.x), (this->y + other.y), (this->z + other.z));
	}

	FVector operator-(const FVector& other) const
	{
		return FVector((this->x - other.x), (this->y - other.y), (this->z - other.z));
	}

	FVector operator*(const float scalar) const
	{
		return FVector((this->x * scalar), (this->y * scalar), (this->z * scalar));
	}

	FVector operator/(const float scalar) const
	{
		return FVector((this->x / scalar), (this->y / scalar), (this->z / scalar));
	}

	FVector& operator+=(const FVector& other)
	{
		this->x += other.x;
		this->y += other.y;
		this->z += other.z;
		return *this;
	}

	FVector& operator-=(const FVector& other)
	{
		this->x -= other.x;
		this->y -= other.y;
		this->z -= other.z;
		return *this;
	}
	FVector& operator*=(const float scalar)
	{
		this->x *= scalar;
		this->y *= scalar;
		this->z *= scalar;
		return *this;
	}
	FVector& operator/=(const float scalar)
	{
		this->x /= scalar;
		this->y /= scalar;
		this->z /= scalar;
		return *this;
	}

	static float Dot(const FVector& lhs, const FVector& rhs)
	{
		return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
	}

	static FVector Cross(const FVector& lhs, const FVector& rhs)
	{
		return FVector(
			(lhs.y * rhs.z) - (lhs.z * rhs.y),
			(lhs.z * rhs.x) - (lhs.x * rhs.z),
			(lhs.x * rhs.y) - (lhs.y * rhs.x)
		);
	}

	float Length() const
	{
		float length = FVector::Dot(*this, *this);
		return sqrtf(length);
	}

	float LenghtSq() const
	{
		return FVector::Dot(*this, *this);
	}

	FVector& Normalize()
	{
		// 자신을 정규화
		float length = this->Length();
		if (length > 1e-6)
		{
			*this /= length;
		}
		return *this;
	}

	FVector Normalize() const
	{
		// 정규화된 복사본 반환
		float length = this->Length();
		if (length > 1e-6)
		{
			return (*this) / length;
		}
		return FVector(0.0f, 0.0f, 0.0f);
	}
};

struct FVector4
{
	float x, y, z, w;

	FVector4(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f) :x(_x), y(_y), z(_z), w(_w) {};

	// 복사 생성자, 복사 대입 연산자는 컴파일러 생성에 맡김

	FVector4 operator+(const FVector4& other) const
	{
		return FVector4((this->x + other.x), (this->y + other.y), (this->z + other.z), (this->w + other.w));
	}

	FVector4 operator-(const FVector4& other) const
	{
		return FVector4((this->x - other.x), (this->y - other.y), (this->z - other.z), (this->w - other.w));
	}

	FVector4 operator*(const float scalar) const
	{
		return FVector4((this->x * scalar), (this->y * scalar), (this->z * scalar), (this->w * scalar));
	}

	FVector4 operator/(const float scalar) const
	{
		return FVector4((this->x / scalar), (this->y / scalar), (this->z / scalar), (this->w / scalar));
	}

	FVector4& operator+=(const FVector4& other)
	{
		this->x += other.x;
		this->y += other.y;
		this->z += other.z;
		this->w += other.w;
		return *this;
	}

	FVector4& operator-=(const FVector4& other)
	{
		this->x -= other.x;
		this->y -= other.y;
		this->z -= other.z;
		this->w -= other.w;
		return *this;
	}
	FVector4& operator*=(const float scalar)
	{
		this->x *= scalar;
		this->y *= scalar;
		this->z *= scalar;
		this->w *= scalar;
		return *this;
	}
	FVector4& operator/=(const float scalar)
	{
		this->x /= scalar;
		this->y /= scalar;
		this->z /= scalar;
		this->w /= scalar;
		return *this;
	}

	static float Dot(const FVector4& lhs, const FVector4& rhs)
	{
		return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z) + (lhs.w * rhs.w);
	}

	float Length() const
	{
		float length = FVector4::Dot(*this, *this);
		return sqrtf(length);
	}

	float LenghtSq() const
	{
		return FVector4::Dot(*this, *this);
	}

	FVector4& Normalize()
	{
		// 자신을 정규화
		float length = this->Length();
		if (length > 1e-6)
		{
			*this /= length;
		}
		return *this;
	}

	FVector4 Normalize() const
	{
		// 정규화된 복사본 반환
		float length = this->Length();
		if (length > 1e-6)
		{
			return (*this) / length;
		}
		return FVector4(0.0f, 0.0f, 0.0f, 0.0f);
	}
};