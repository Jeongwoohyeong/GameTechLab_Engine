#include "UCamera.h"

void UCamera::SetCameraPosition(const float x, const float y, const float z)
{
	// 월드 스페이스에서의 위치 - 월드의 원점에서 얼마나 이동했는지
	Position = { x, y, z };
}

void UCamera::SetCameraPosition(const FVector& position)
{
	Position = position;
}

void UCamera::SetCameraRotation(const float degreeX, const float degreeY, const float degreeZ)
{
	Rotation = { (degreeX * (3.1415926f / 180.0f)), (degreeY * (3.1415926f / 180.0f)), (degreeZ * (3.1415926f / 180.0f)) };
}

void UCamera::SetCameraRotation(const FVector& degrees)
{
	Rotation = { (degrees.x * (3.1415926f / 180.0f)), (degrees.y * (3.1415926f / 180.0f)), (degrees.z * (3.1415926f / 180.0f)) };
}

void UCamera::CreateMVPMatrix()
{
	FVector eyePosition = { -Position.x, -Position.y, -Position.z };

	FMatrix translationMatrix = FMatrix::CreateTranslation(eyePosition);
	FMatrix rotationMatrix = FMatrix::CreateFromYawPitchRoll(Rotation.z, Rotation.x, Rotation.y).Transpose();
	
	ViewMatrix = translationMatrix * rotationMatrix;

}