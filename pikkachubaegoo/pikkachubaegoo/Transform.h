#pragma once

#include "Matrix.h"

class Transform
{
public:
	Transform();
	~Transform();

	void SetScale(float x, float y, float z);
	void SetScale(const FVector2&);
	void SetScale(const FVector3&);

	void SetRotationX(float degree);
	void SetRotationY(float degree);
	void SetRotationZ(float degree);

	void AddRotationX(float degree);
	void AddRotationY(float degree);
	void AddRotationZ(float degree);

	void SetLocation(float x, float y, float z);
	void SetLocation(const FVector2&);
	void SetLocation(const FVector3&);

	void Translate(float dx, float dy, float dz);
	void Translate(const FVector2&);
	void Translate(const FVector3&);

	const FVector3& GetScale() const { return scale; }
	const FVector3& GetRotationRadians() const { return rotation; }
	FVector3 GetRotationDegree() const;
	const FVector3& GetLocation() const { return location; }

	const FMatrix4x4& GetTransformMatrix();
private:
	float DegreeToRadians(float degree) const;
	float RadiansToDegree(float radians) const;

private:
	FMatrix4x4 transform;
	FVector3 scale;
	FVector3 rotation;
	FVector3 location;

	static constexpr float PI = 3.1415926f;

	// 트랜스폼 변경 검사
	bool isDirty;
};