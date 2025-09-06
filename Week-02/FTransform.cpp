#include "FTransform.h"

FTransform::FTransform()
{
	Transform = FMatrix::CreateIdentity();
	Scale = (1.0f, 1.0f, 1.0f);
	Rotation = FVector(0.0f, 0.0f, 0.0f);
	Location = FVector(0.0f, 0.0f, 0.0f);
	isDirty = true;
}

FTransform::~FTransform()
{
}

void FTransform::SetScale(float x, float y, float z)
{
	Scale = FVector(x, y, z);

	isDirty = true;
}

void FTransform::SetScale(const FVector& scale2d)
{
	Scale = FVector(scale2d.x, scale2d.y, 1.0f);

	isDirty = true;
}

void FTransform::SetRotationX(float degree)
{
	Rotation.x = DegreeToRadians(degree);

	isDirty = true;
}

void FTransform::SetRotationY(float degree)
{
	Rotation.y = DegreeToRadians(degree);

	isDirty = true;
}

void FTransform::SetRotationZ(float degree)
{
	Rotation.z = DegreeToRadians(degree);

	isDirty = true;
}

void FTransform::SetRotation(const FVector& degrees)
{
	this->SetRotationX(degrees.x);
	this->SetRotationY(degrees.y);
	this->SetRotationZ(degrees.z);

	isDirty = true;
}

void FTransform::AddRotationX(float degree)
{
	Rotation.x += DegreeToRadians(degree);

	isDirty = true;
}

void FTransform::AddRotationY(float degree)
{
	Rotation.y += DegreeToRadians(degree);

	isDirty = true;
}

void FTransform::AddRotationZ(float degree)
{
	Rotation.z += DegreeToRadians(degree);

	isDirty = true;
}

void FTransform::SetLocation(float x, float y, float z)
{
	Location = FVector(x, y, z);

	isDirty = true;
}

void FTransform::SetLocation(const FVector& location2d)
{
	Location = FVector(location2d.x, location2d.y, 0.0f);

	isDirty = true;
}

void FTransform::Translate(float dx = 0.0f, float dy = 0.0f, float dz = 0.0f)
{
	Location = Location + FVector(dx, dy, dz);

	isDirty = true;
}

void FTransform::Translate(const FVector& offset)
{
	Location = Location + offset;

	isDirty = true;
}

FVector FTransform::GetRotationDegree() const
{
	return FVector(
		RadiansToDegree(Rotation.x),
		RadiansToDegree(Rotation.y),
		RadiansToDegree(Rotation.z)
	);
}

const FMatrix& FTransform::GetInverseMatrix()
{
	if (isDirty)
	{
		FMatrix scaleMatrix = FMatrix::CreateScale(Scale.x, Scale.y, Scale.z);
		FMatrix rotationMatrix = FMatrix::CreateFromYawPitchRoll(Rotation.y, Rotation.x, Rotation.z);
		FMatrix translationMatrix = FMatrix::CreateTranslation(Location.x, Location.y, Location.z);

		Inverse = translationMatrix * rotationMatrix * scaleMatrix;

		isDirty = false;
	}

	return Inverse;
}

const FMatrix& FTransform::GetTransformMatrix()
{
	// SRT중 하나라도 변경되면 행렬 연산
	if (isDirty)
	{
		FMatrix scaleMatrix = FMatrix::CreateScale(Scale.x, Scale.y, Scale.z);
		FMatrix rotationMatrix = FMatrix::CreateFromYawPitchRoll(Rotation.y, Rotation.x, Rotation.z);
		FMatrix translationMatrix = FMatrix::CreateTranslation(Location.x, Location.y, Location.z);

		Transform = scaleMatrix * rotationMatrix * translationMatrix;

		isDirty = false;
	}

	return Transform;
}

float FTransform::DegreeToRadians(float degree) const
{
	float rad = degree * (PI / 180.0f);

	return rad;
}

float FTransform::RadiansToDegree(float radians) const
{
	float deg = radians * (180.0f / PI);
	while (deg <= -360)
	{
		deg += 360;
	}
	while (deg >= 360)
	{
		deg -= 360;
	}
	return deg;
}