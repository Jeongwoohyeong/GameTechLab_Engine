#pragma once

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
};