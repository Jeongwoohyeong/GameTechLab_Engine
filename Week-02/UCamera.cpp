#include "UCamera.h"

void UCamera::Init()
{
	Location = { 0.0f, 0.0f, -5.0f };
	Rotation = { 0.0f, 0.0f, 0.0f };
	/*Target = { 0.0f, 0.0f, 0.0f };
	Up = { 0.0f, 1.0f, 0.0f };*/
	FovY = DegToRad(60.0f);
	AspectRatio = 1.0f;
	NearPlane = 0.1f;
	FarPlane = 100.0f;
}

FMatrix UCamera::MakeMVP(const FMatrix& World)
{
	FMatrix View = FMatrix::MakeView(Location, Rotation);
	FMatrix Projection = FMatrix::MakePerspective(FovY, AspectRatio, NearPlane, FarPlane);

	return World * View * Projection;
}

// 클라이언트 화면 상에서의 커서 좌표를 이용해 3D 공간상의 레이를 만年求.
FRay UCamera::CastRay(int ClientX, int ClientY, int ClientWidth, int ClientHeight)
{
	// 클라이언트 좌표계: 좌상단 (0,0) ~ 우하단 (width,height)
	// NCD: 좌하단 (-1,-1) ~ 우상단 (1,1)

	// 1. 클라이언트 좌표 -> NDC 좌표 변환
	float NDCX = (2.0f * ClientX) / ClientWidth - 1.0f;
	float NDCY = (2.0f * ClientY) / ClientHeight - 1.0f;
	NDCY = -NDCY;

	// 2. NDC -> 카메라 공간 상에서 Ray 방향 벡터 계산
	float TanFovY = tan(0.5f * FovY);
	// projection plane이 z = 1 평면에 있다고 가정
	float CameraX = NDCX * AspectRatio * TanFovY; 
	float CameraY = NDCY * TanFovY;
	FVector RayDirCameraSpace = FVector(CameraX, CameraY, 1.0f).GetNormalized();

	// 3. 카메라 공간 -> 월드 공간 상에서 Ray 방향 벡터 계산
	FMatrix CamRotMat = FMatrix::MakeRotation(Rotation);
	FMatrix CamPosMat = FMatrix::MakeTranslation(Location);
	FMatrix RayToWorldMat = FMatrix::Identity() * CamRotMat * CamPosMat;
	FVector4 RayDirCameraSpace4(RayDirCameraSpace, 0.0f);
	FVector4 RayDirWorldSpace4 = RayDirCameraSpace4 * RayToWorldMat;
	FVector RayDirWorldSpace(RayDirWorldSpace4.X, RayDirWorldSpace4.Y, RayDirWorldSpace4.Z);
	RayDirCameraSpace.Normalize();

	return FRay(Location, RayDirWorldSpace);
}