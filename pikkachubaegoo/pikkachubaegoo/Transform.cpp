#include <cmath>
#include "Transform.h"

Transform::Transform()
{
	transform = FMatrix4x4::CreateIdentity();
	scale = FVector3(1.0f, 1.0f, 1.0f);
	rotation = FVector3(0.0f, 0.0f, 0.0f);
	location = FVector3(0.0f, 0.0f, 0.0f);
	isDirty = true;
}

Transform::~Transform()
{
}

void Transform::SetScale(float x, float y, float z)
{
	scale = FVector3(x, y, z);

	isDirty = true;
}

void Transform::SetScale(const FVector2& scale2d)
{
	scale = FVector3(scale2d.x, scale2d.y, 1.0f);

	isDirty = true;
}

void Transform::SetScale(const FVector3& scale3d)
{
	scale = scale3d;

	isDirty = true;
}

void Transform::SetRotationX(float degree)
{
	rotation.x = DegreeToRadians(degree);

	isDirty = true;
}

void Transform::SetRotationY(float degree)
{
	rotation.y = DegreeToRadians(degree);

	isDirty = true;
}

void Transform::SetRotationZ(float degree)
{
	rotation.z = DegreeToRadians(degree);

	isDirty = true;
}

void Transform::SetPosition(float x, float y, float z)
{
	location = FVector3(x, y, z);

	isDirty = true;
}

void Transform::SetPosition(const FVector2& location2d)
{
	location = FVector3(location2d.x, location2d.y, 0.0f);

	isDirty = true;
}

void Transform::SetPosition(const FVector3& location3d)
{
	location = location3d;

	isDirty = true;
}

void Transform::Translate(float dx, float dy, float dz)
{
	location = location + FVector3(dx, dy, dz);

	isDirty = true;
}

void Transform::Translate(const FVector2& offset2d)
{
	location = location + FVector3(offset2d.x, offset2d.y, 0.0f);

	isDirty = true;
}

void Transform::Translate(const FVector3& offset3d)
{
	location = location + offset3d;

	isDirty = true;
}

const FMatrix4x4& Transform::GetTransform()
{
	// SRT중 하나라도 변경되면 행렬 연산
	if (isDirty)
	{
		FMatrix4x4 scaleMatrix = FMatrix4x4::CreateScale(scale.x, scale.y, scale.z);
		FMatrix4x4 rotationMatrix = FMatrix4x4::CreateFromYawPitchRoll(rotation.y, rotation.x, rotation.z);
		FMatrix4x4 translationMatrix = FMatrix4x4::CreateTranslation(location.x, location.y, location.z);

		transform = scaleMatrix * rotationMatrix * translationMatrix;

		isDirty = false;
	}

	return transform;
}

float Transform::DegreeToRadians(float degree)
{
	float rad = degree * (PI / 180.0f);

	return rad;
}

float Transform::RadiansToDegree(float radians)
{
	float deg = radians * (180.0f / PI);

	return 0.0f;
}
