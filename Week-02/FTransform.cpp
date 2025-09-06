#include "FTransform.h"

FTransform::FTransform()
{
	Transform = FMatrix::Identity();
	Scale = FVector(1.0f, 1.0f, 1.0f);
	Rotation = FVector(0.0f, 0.0f, 0.0f);
	Location = FVector(0.0f, 0.0f, 0.0f);
	Inverse = {};
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

void FTransform::SetScale(const FVector& scale)
{
	Scale = FVector(scale.X, scale.Y, scale.Z);

	isDirty = true;
}

void FTransform::SetRotationX(float degree)
{
	Rotation.X = DegToRad(degree);

	isDirty = true;
}

void FTransform::SetRotationY(float degree)
{
	Rotation.Y = DegToRad(degree);

	isDirty = true;
}

void FTransform::SetRotationZ(float degree)
{
	Rotation.Z = DegToRad(degree);

	isDirty = true;
}

void FTransform::SetRotation(const FVector& degrees)
{
	this->SetRotationX(degrees.X);
	this->SetRotationY(degrees.Y);
	this->SetRotationZ(degrees.Z);

	isDirty = true;
}

void FTransform::AddRotationX(float degree)
{
	Rotation.X += DegToRad(degree);

	isDirty = true;
}

void FTransform::AddRotationY(float degree)
{
	Rotation.Y += DegToRad(degree);

	isDirty = true;
}

void FTransform::AddRotationZ(float degree)
{
	Rotation.Z += DegToRad(degree);

	isDirty = true;
}

void FTransform::SetLocation(float x, float y, float z)
{
	Location = FVector(x, y, z);

	isDirty = true;
}

void FTransform::SetLocation(const FVector& location)
{
	Location = FVector(location.X, location.X, location.Y);

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
		RadToDeg(Rotation.X),
		RadToDeg(Rotation.Y),
		RadToDeg(Rotation.Z)
	);
}

const FMatrix& FTransform::GetInverseMatrix()
{
	if (isDirty)
	{
		FMatrix scaleMatrix = FMatrix::MakeScale(Scale);
		FMatrix rotationMatrix = FMatrix::MakeRotation(Rotation);
		FMatrix translationMatrix = FMatrix::MakeTranslation(Location);

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
		FMatrix scaleMatrix = FMatrix::MakeScale(Scale);
		FMatrix rotationMatrix = FMatrix::MakeRotation(Rotation);
		FMatrix translationMatrix = FMatrix::MakeTranslation(Location);

		Transform = scaleMatrix * rotationMatrix * translationMatrix;

		isDirty = false;
	}

	return Transform;
}