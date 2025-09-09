#pragma once
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
	float AspectRatio; // 종횡비 (화면 너비 / 높이)
	float NearPlane; // 근평면
	float FarPlane; // 원평면
	bool bIsOrthogonal = false;

	// 싱글톤 인스턴스 접근
	static UCamera& GetInstance()
	{
		static UCamera instance;
		return instance;
	}

	void Init();

	// MVP 행렬 계산
	FMatrix MakeMVP(const FMatrix& World);

	FRay CastRay(int screenX, int screenY, int screenWidth, int screenHeight);
};