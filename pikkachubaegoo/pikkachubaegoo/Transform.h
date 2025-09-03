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

	void SetPosition(float x, float y, float z);
	void SetPosition(const FVector2&);
	void SetPosition(const FVector3&);

	void Translate(float dx, float dy, float dz);
	void Translate(const FVector2&);
	void Translate(const FVector3&);

	const FMatrix4x4& GetTransform();
	

private:
	float DegreeToRadians(float degree);
	float RadiansToDegree(float radians);

private:
	FMatrix4x4 transform;
	FVector3 scale;
	FVector3 rotation;
	FVector3 location;

	const float PI = 3.1415926f;

	// 트랜스폼 변경 검사
	bool isDirty;
};