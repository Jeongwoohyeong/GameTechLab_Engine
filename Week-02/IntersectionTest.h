#pragma once
#include "types.h"
#include "Math.h"
#include "FTransform.h"
#include "FRay.h"

struct FAABB
{
	FVector Min;
	FVector Max;
};

struct FHit {
    float T;          // 선택된 t (enter>=0 이면 enter, 아니면 exit 등)
    float TEnter;     // 교집합 시작
    float TExit;      // 교집합 끝
    bool bIsInside;     // 레이 원점이 박스 내부였는지

    FVector PointLocal; // 로컬 히트 위치
    FVector NormalLocal;// 로컬 히트 노멀(±axis[i])
    FVector PointWorld; // 월드 히트 위치(옵션)
    FVector NormalWorld;// 월드 히트 노멀(옵션)
};

bool CheckIntersectionRayBox(const FRay& Ray, const FAABB& LocalBox, const FTransform& T);
