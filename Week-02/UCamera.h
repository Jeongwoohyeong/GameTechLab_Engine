#pragma once
#include "Types.h"
#include "Math.h"
#include "FRay.h"

class UCamera
{
public:
	FVector Location; // 카메라 위치
	FVector Rotation; // 카메라 회전 (오일러 각)
	//FVector Target; // 카메라 타겟 (바라보는 지점)
	//FVector Up; // 카메라 업 벡터
	float FovY; // 시야각 (Field of View)
	float Width;
	float Height;
	float NearPlane = 0.01f; // 근평면
	float FarPlane; // 원평면
	bool bIsOrthogonal = false;

	// 싱글톤 인스턴스 접근
	static UCamera& GetInstance()
	{
		static UCamera instance;
		return instance;
	}

	void Init(FVector Loc, FVector Rot, float FovDeg, float Width, float Height);

	// MVP 행렬 계산
	FMatrix MakeMVP(const FMatrix& World);

	FMatrix MakeGizmoMVP(const FMatrix& world, const FVector& gizmoLocation);

	// 마우스 포인터 위치에 해당하는 Projection Plane 상의 월드 좌표 반환
	// @param Depth: 카메라로부터의 거리 (Projection Plane이 View Space에서 z = Depth 평면에 있다고 가정)
	// @param bWorld: 월드 좌표계로 반환할지 여부 (true: 월드 좌표, false: 카메라 공간 좌표)
	FVector DeprojectScreenPoint(int32 ClientX, int32 ClientY, int32 ClientWidth, int32 ClientHeight, float Depth = 1.0f, bool bWorld = false);

	FRay CastRay(int32 screenX, int32 screenY, int32 screenWidth, int32 screenHeight); // Ray만 필요한 경우
};