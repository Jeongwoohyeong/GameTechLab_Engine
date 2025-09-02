#pragma once

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
