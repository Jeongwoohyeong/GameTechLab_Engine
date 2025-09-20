#include "pch.h"
#include "Components/StaticMeshComponent.h"
#include "Level/Level.h"

IMPLEMENT_CLASS(UStaticMeshComponent, UMeshComponent)


//StaticMesh가 구현되면 주석 해제(09/19 13:05)
void UStaticMeshComponent::AddToRenderList(ULevel* Level)
{
	Level->AddStaticMeshComponentToRender(this);
}

bool UStaticMeshComponent::IsRayCollided(const FRay& ModelRay, const FMatrix& ModelMatrix, float* ShortestDistance)
{

	FStaticMesh* StaticMeshAsset = StaticMesh->GetStaticMeshAsset();

	const TArray<uint32>& Indices = StaticMeshAsset->Indices;
	const TArray<FNormalVertex>& Vertices = StaticMeshAsset->Vertices;

	float Distance = D3D11_FLOAT32_MAX; //Distance 초기화
	bool bIsHit = false;
	for (int32 a = 0; a < Indices.Num(); a = a + 3) //삼각형 단위로 Vertex 위치정보 읽음
	{
		const FVector& Vertex1 = Vertices[Indices[a]].Position;
		const FVector& Vertex2 = Vertices[Indices[a+1]].Position;
		const FVector& Vertex3 = Vertices[Indices[a+2]].Position;

		if (IsRayTriangleCollided(ModelRay, Vertex1, Vertex2, Vertex3, ModelMatrix, &Distance)) //Ray와 삼각형이 충돌하면 거리 비교 후 최단거리 갱신

		{
			bIsHit = true;
			if (Distance < *ShortestDistance)
			{
				*ShortestDistance = Distance;
			}
		}
	}

	return bIsHit;
}

bool UStaticMeshComponent::IsRayTriangleCollided(const FRay& Ray, const FVector& Vertex1, const FVector& Vertex2, const FVector& Vertex3,
	const FMatrix& ModelMatrix, float* Distance)
{


	//삼각형 내의 점은 E1*V + E2*U + Vertex1.Position으로 표현 가능( 0<= U + V <=1,  Y>=0, V>=0 )
	//Ray.Direction * T + Ray.Origin = E1*V + E2*U + Vertex1.Position을 만족하는 T U V값을 구해야 함.
	//[E1 E2 RayDirection][V U T] = [RayOrigin-Vertex1.Position]에서 cramer's rule을 이용해서 T U V값을 구하고
	//U V값이 저 위의 조건을 만족하고 T값이 카메라의 near값 이상이어야 함.
	FVector RayDirection{ Ray.Direction.X, Ray.Direction.Y, Ray.Direction.Z };
	FVector RayOrigin{ Ray.Origin.X, Ray.Origin.Y, Ray.Origin.Z };
	FVector E1 = Vertex2 - Vertex1;
	FVector E2 = Vertex3 - Vertex1;
	FVector Result = (RayOrigin - Vertex1); //[E1 E2 -RayDirection]x = [RayOrigin - Vertex1.Position] 의 result임.


	FVector CrossE2Ray = E2.Cross(RayDirection);
	FVector CrossE1Result = E1.Cross(Result);

	float Determinant = E1.Dot(CrossE2Ray);

	float NoInverse = 0.0001f; //0.0001이하면 determinant가 0이라고 판단=>역행렬 존재 X
	if (abs(Determinant) <= NoInverse)
	{
		return false;
	}


	float V = Result.Dot(CrossE2Ray) / Determinant; //cramer's rule로 해를 구했음. 이게 0미만 1초과면 충돌하지 않음.

	if (V < 0 || V > 1)
	{
		return false;
	}

	float U = RayDirection.Dot(CrossE1Result) / Determinant;
	if (U < 0 || U + V > 1)
	{
		return false;
	}


	float T = E2.Dot(CrossE1Result) / Determinant;

	FVector HitPoint = RayOrigin + RayDirection * T; //모델 좌표계에서의 충돌점
	FVector4 HitPoint4{ HitPoint.X, HitPoint.Y, HitPoint.Z, 1 };
	FVector4 DistanceVec = HitPoint4 - Ray.Origin;
	if (T > 0)
	{
		*Distance = DistanceVec.Length();
		return true;
	}
	//이제 이것을 월드 좌표계로 변환해서 view Frustum안에 들어가는지 판단할 것임.(near, far plane만 테스트하면 됨)

	/*FVector4 HitPointWorld = HitPoint4 * ModelMatrix;
	FVector4 RayOriginWorld = Ray.Origin * ModelMatrix;

	FVector4 DistanceVec = HitPointWorld - RayOriginWorld;
	if (DistanceVec.Dot3(CameraForward) >= NearZ && DistanceVec.Dot3(CameraForward) <= FarZ)
	{
		*Distance = DistanceVec.Length();
		return true;
	}*/
	return false;
}



//TODO: Serialize 구현
//void UStaticMeshComponent::Serialize(bool bIsLoading, json::JSON Handle)
//{
//	//부모 객체부터 Serialize가 필요한데 일단 패스
//	//Super::Serialize(bIsLoading, Handle);
//
//	/*if (bIsLoading)
//	{
//		FString assetName;
//		Handle << "ObjStaticMeshAsset" << assetName;
//		StaticMesh = FObjManager::LoadObjStaticMesh(assetName);
//	}
//	else
//	{
//		FString assetName = StaticMesh->GetAssetPathFileName();
//		Handle << "ObjStaticMeshAsset" << assetName;
//	}*/
//}


