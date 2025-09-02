#pragma once

struct FVector {
	float x, y, z;
	FVector(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) : x(_x), y(_y), z(_z) {}

	FVector operator+(const FVector& other) const
	{
		return FVector(x + other.x, y + other.y, z + other.z);
	}

	FVector operator-(const FVector& other) const
	{
		return FVector(x - other.x, y - other.y, z - other.z);
	}

	FVector operator*(float scalar) const
	{
		return FVector(x * scalar, y * scalar, z * scalar);
	}

	FVector operator/(float scalar) const
	{
		return FVector(x / scalar, y / scalar, z / scalar);
	}
};
