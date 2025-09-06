#include "UCamera.h"

void UCamera::Init()
{
	Location = { 0.0f, 0.0f, -10.0f };
	Rotation = { 0.0f, 0.0f, 0.0f };
	Target = { 0.0f, 0.0f, 0.0f };
	Up = { 0.0f, 1.0f, 0.0f };
	FOV = DegToRad(60.0f);
	AspectRatio = 1.0f;
	NearPlane = 0.1f;
	FarPlane = 100.0f;
}

FMatrix UCamera::MakeMVP(const FMatrix& World)
{
	FMatrix View = FMatrix::MakeView(Location, Rotation);
	FMatrix Projection = FMatrix::MakePerspectiveFovLH(FOV, AspectRatio, NearPlane, FarPlane);

	return World * View * Projection;
}