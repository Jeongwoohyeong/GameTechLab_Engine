#pragma once

struct FVector4 {
	float x, y, z, w;
	FVector4(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f) : x(_x), y(_y), z(_z), w(_w) {}

	FVector4 operator+(const FVector4& other) const
	{
		return FVector4(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	FVector4 operator-(const FVector4& other) const
	{
		return FVector4(x - other.x, y - other.y, z - other.z, w - other.w);
	}

	FVector4 operator*(float scalar) const
	{
		return FVector4(x * scalar, y * scalar, z * scalar, w * scalar);
	}

	FVector4 operator/(float scalar) const
	{
		return FVector4(x / scalar, y / scalar, z / scalar, w / scalar);
	}
};

struct FVector3 {
	float x, y, z;
	FVector3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) : x(_x), y(_y), z(_z) {}

	FVector3 operator+(const FVector3& other) const
	{
		return FVector3(x + other.x, y + other.y, z + other.z);
	}

	FVector3 operator-(const FVector3& other) const
	{
		return FVector3(x - other.x, y - other.y, z - other.z);
	}

	FVector3 operator*(float scalar) const
	{
		return FVector3(x * scalar, y * scalar, z * scalar);
	}

	FVector3 operator/(float scalar) const
	{
		return FVector3(x / scalar, y / scalar, z / scalar);
	}
};
struct FVector2 {
	float x, y;
	FVector2(float _x = 0.0f, float _y = 0.0f) : x(_x), y(_y){}
	FVector2(const FVector3& other) : x(other.x), y(other.y) {}

	FVector2 operator+(const FVector2& other) const
	{
		return FVector2(x + other.x, y + other.y);
	}

	FVector2 operator-(const FVector2& other) const
	{
		return FVector2(x - other.x, y - other.y);
	}

	FVector2 operator*(float scalar) const
	{
		return FVector2(x * scalar, y * scalar);
	}

	FVector2 operator/(float scalar) const
	{
		return FVector2(x / scalar, y / scalar);
	}
};
