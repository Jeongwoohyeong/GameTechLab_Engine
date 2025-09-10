#include "UCamera.h"

void UCamera::Init(FVector Loc, FVector Rot, float FovDeg, float Ratio)
{
	Location = Loc;
	Rotation = Rot;
	/*Target = { 0.0f, 0.0f, 0.0f };
	Up = { 0.0f, 1.0f, 0.0f };*/
	FovY = DegToRad(FovDeg);
	AspectRatio = Ratio;
	NearPlane = 0.1f;
	FarPlane = 100.0f;
}

FMatrix UCamera::MakeMVP(const FMatrix& World)
{
	FMatrix View = FMatrix::MakeView(Location, Rotation);
	FMatrix Projection = {};
	if (bIsOrthogonal)
	{
		Projection = FMatrix::MakeOrthographic((-15.0f * AspectRatio / 2.0f), (15.0f * AspectRatio / 2.0f),
			(-15.0f / 2.0f), (15.0f / 2.0f), NearPlane, FarPlane);
	}
	else
	{
		Projection = FMatrix::MakePerspective(FovY, AspectRatio, NearPlane, FarPlane);
	}

	return World * View * Projection;
}

// 클라이언트 화면 상에서의 커서 좌표를 이용해 3D 공간상의 레이를 만든다
FRay UCamera::CastRay(int32 ClientX, int32 ClientY, int32 ClientWidth, int32 ClientHeight)
{
	FVector DeprojPoint = DeprojectScreenPoint(ClientX, ClientY, ClientWidth, ClientHeight, 1.0f, false); // View Space 기준
	FVector RayDir = DeprojPoint.GetNormalized();

	// 카메라 공간 -> 월드 공간 상에서 Ray 방향 벡터 계산
	FMatrix CamRotMat = FMatrix::MakeRotation(Rotation);
	FVector4 RayDirCameraSpace4(RayDir, 0.0f);
	FVector4 RayDirWorldSpace4 = RayDirCameraSpace4 * CamRotMat; // 순수 vector이므로 회전 변환만 수행
	FVector RayDirWorldSpace(RayDirWorldSpace4.X, RayDirWorldSpace4.Y, RayDirWorldSpace4.Z);
	RayDirWorldSpace.Normalize();

	return FRay(Location, RayDirWorldSpace);
}

// 마우스 포인터 위치에 해당하는 Projection Plane 상의 월드 좌표 반환
// @param Depth: 카메라로부터의 거리 (Projection Plane이 View Space에서 z = Depth 평면에 있다고 가정)
// @param bWorld: 월드 좌표계로 반환할지 여부 (true: 월드 좌표, false: 카메라 공간 좌표)
FVector UCamera::DeprojectScreenPoint(int32 ClientX, int32 ClientY, int32 ClientWidth, int32 ClientHeight, float Depth, bool bWorld)
{
	// Note:
	// Perspective Camera 기준
	// Projection 행렬의 역행렬을 사용하는 방법은 NearPlane, FarPlane에 점을 잡지만
	// 현 프로젝트에서는 Eye와 Deproject Point 하나를 사용

	// 클라이언트 좌표 -> NDC 좌표 변환
	float NDCX = (2.0f * ClientX) / ClientWidth - 1.0f;
	float NDCY = (2.0f * ClientY) / ClientHeight - 1.0f;
	NDCY = -NDCY;
	
	float TanFovY = tan(0.5f * FovY);
	// projection plane이 z = Depth 평면에 있다고 가정
	float CameraX = NDCX * AspectRatio * TanFovY * Depth;
	float CameraY = NDCY * TanFovY * Depth;
	FVector DeprojectedPoint = FVector(CameraX, CameraY, Depth);

	if(bWorld)
	{
		// 월드 좌표계로 변환
		FMatrix CamRotMat = FMatrix::MakeRotation(Rotation);
		FMatrix CamPosMat = FMatrix::MakeTranslation(Location);
		FMatrix CamWorldTransformMat = FMatrix::Identity() * CamRotMat * CamPosMat;
		FVector4 DeprojectedPoint4 = FVector4(DeprojectedPoint, 1.0f);
		DeprojectedPoint4 = DeprojectedPoint4 * CamWorldTransformMat;
		DeprojectedPoint = FVector(DeprojectedPoint4.X, DeprojectedPoint4.Y, DeprojectedPoint4.Z);
	}

	return DeprojectedPoint;
}