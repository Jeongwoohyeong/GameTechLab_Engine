#include "pch.h"
#include "Component/Public/SceneComponent.h"
#include "Manager/Asset/Public/AssetManager.h"
#include "Utility/Public/JsonSerializer.h"

#include "Component/Public/PrimitiveComponent.h"
#include "Level/Public/Level.h"

#include <json.hpp>

IMPLEMENT_CLASS(USceneComponent, UActorComponent)

USceneComponent::USceneComponent()
{
}

void USceneComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USceneComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
}

void USceneComponent::EndPlay()
{
	Super::EndPlay();
}
void USceneComponent::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
	Super::Serialize(bInIsLoading, InOutHandle);

	// 불러오기
	if (bInIsLoading)
	{
		FJsonSerializer::ReadVector(InOutHandle, "Location", RelativeLocation, FVector::ZeroVector());
		FVector RotationEuler;
		FJsonSerializer::ReadVector(InOutHandle, "Rotation", RotationEuler, FVector::ZeroVector());
		RelativeRotation = FQuaternion::FromEuler(RotationEuler);

		FJsonSerializer::ReadVector(InOutHandle, "Scale", RelativeScale3D, FVector::OneVector());
	}
	// 저장
	else
	{
		InOutHandle["Location"] = FJsonSerializer::VectorToJson(RelativeLocation);
		InOutHandle["Rotation"] = FJsonSerializer::VectorToJson(RelativeRotation.ToEuler());
		InOutHandle["Scale"] = FJsonSerializer::VectorToJson(RelativeScale3D);
	}
}

void USceneComponent::AttachToComponent(USceneComponent* Parent, bool bRemainTransform)
{
	if (!Parent || Parent == this || GetOwner() != Parent->GetOwner()) { return; }
	if (AttachParent)
	{
		AttachParent->DetachChild(this);
	}

	AttachParent = Parent;
	Parent->AttachChildren.push_back(this);

	MarkAsDirty();
}

void USceneComponent::DetachFromComponent()
{
	if (AttachParent)
	{
		AttachParent->DetachChild(this);
		AttachParent = nullptr;
	}
}

void USceneComponent::DetachChild(USceneComponent* ChildToDetach)
{
	AttachChildren.erase(std::remove(AttachChildren.begin(), AttachChildren.end(), ChildToDetach), AttachChildren.end());
}

UObject* USceneComponent::Duplicate()
{
	USceneComponent* SceneComponent = Cast<USceneComponent>(Super::Duplicate());
	SceneComponent->RelativeLocation = RelativeLocation;
	SceneComponent->RelativeRotation = RelativeRotation;
	SceneComponent->RelativeScale3D = RelativeScale3D;
	SceneComponent->MarkAsDirty();
	return SceneComponent;
}

void USceneComponent::DuplicateSubObjects(UObject* DuplicatedObject)
{
	Super::DuplicateSubObjects(DuplicatedObject);
}

void USceneComponent::MarkAsDirty()
{
	bIsTransformDirty = true;
	bIsTransformDirtyInverse = true;

	for (USceneComponent* Child : AttachChildren)
	{
		Child->MarkAsDirty();
	}
}

void USceneComponent::SetRelativeLocation(const FVector& Location)
{
	RelativeLocation = Location;
	MarkAsDirty();

	if (auto PrimitiveComponent = Cast<UPrimitiveComponent>(this))
	{
		GWorld->GetLevel()->UpdatePrimitiveInOctree(PrimitiveComponent);
	}
}

void USceneComponent::SetRelativeRotation(const FQuaternion& Rotation)
{
	RelativeRotation = Rotation;
	MarkAsDirty();

	if (auto PrimitiveComponent = Cast<UPrimitiveComponent>(this))
	{
		GWorld->GetLevel()->UpdatePrimitiveInOctree(PrimitiveComponent);
	}
}

void USceneComponent::SetRelativeScale3D(const FVector& Scale)
{
	RelativeScale3D = Scale;
	MarkAsDirty();

	if (auto PrimitiveComponent = Cast<UPrimitiveComponent>(this))
	{
		GWorld->GetLevel()->UpdatePrimitiveInOctree(PrimitiveComponent);
	}
}

const FMatrix& USceneComponent::GetWorldTransformMatrix() const
{
	if (bIsTransformDirty)
	{
		WorldTransformMatrix = FMatrix::GetModelMatrix(RelativeLocation, RelativeRotation, RelativeScale3D);
		// Absolute 플래그에 따라 각 컴포넌트를 개별 처리
		FVector WorldLocation = RelativeLocation;
		FQuaternion WorldRotation = RelativeRotation;
		FVector WorldScale = RelativeScale3D;
		if (AttachParent)
		{
			// Row-vector 시스템 (V * M): V * Local * Parent 순서
			// Local 변환 먼저, 그 다음 Parent 변환
			WorldTransformMatrix *= AttachParent->GetWorldTransformMatrix();
			// Location: Absolute가 아니면 부모 변환 적용
			if (!bAbsoluteLocation)
			{
				WorldLocation = AttachParent->GetWorldTransformMatrix().TransformPosition(RelativeLocation);
			}

			// Rotation: Absolute가 아니면 부모 회전 적용
			if (!bAbsoluteRotation)
			{
				WorldRotation = AttachParent->GetWorldRotationAsQuaternion() * RelativeRotation;
			}

			// Scale: Absolute가 아니면 부모 스케일 적용
			if (!bAbsoluteScale)
			{
				const FVector ParentScale = AttachParent->GetWorldScale3D();
				WorldScale = FVector(
					RelativeScale3D.X * ParentScale.X,
					RelativeScale3D.Y * ParentScale.Y,
					RelativeScale3D.Z * ParentScale.Z
				);
			}
		}
		WorldTransformMatrix = FMatrix::GetModelMatrix(WorldLocation, WorldRotation, WorldScale);
		bIsTransformDirty = false;
	}

	return WorldTransformMatrix;
}

const FMatrix& USceneComponent::GetWorldTransformMatrixInverse() const
{
	if (bIsTransformDirtyInverse)
	{
		// (Local * Parent)^-1 = Parent^-1 * Local^-1
		WorldTransformMatrixInverse = FMatrix::Identity();
		// World Transform의 역행렬이므로 GetWorldTransformMatrix의 역순으로 계산
		FVector WorldLocation = RelativeLocation;
		FQuaternion WorldRotation = RelativeRotation;
		FVector WorldScale = RelativeScale3D;
		if (AttachParent)
		{
			WorldTransformMatrixInverse *= AttachParent->GetWorldTransformMatrixInverse();
			// Location: Absolute가 아니면 부모 변환 적용
			if (!bAbsoluteLocation)
			{
				WorldLocation = AttachParent->GetWorldTransformMatrix().TransformPosition(RelativeLocation);
			}

			// Rotation: Absolute가 아니면 부모 회전 적용
			if (!bAbsoluteRotation)
			{
				WorldRotation = AttachParent->GetWorldRotationAsQuaternion() * RelativeRotation;
			}

			// Scale: Absolute가 아니면 부모 스케일 적용
			if (!bAbsoluteScale)
			{
				const FVector ParentScale = AttachParent->GetWorldScale3D();
				WorldScale = FVector(
					RelativeScale3D.X * ParentScale.X,
					RelativeScale3D.Y * ParentScale.Y,
					RelativeScale3D.Z * ParentScale.Z
				);
			}
		}
		WorldTransformMatrixInverse = FMatrix::GetModelMatrixInverse(WorldLocation, WorldRotation, WorldScale);
		bIsTransformDirtyInverse = false;
	}

	return WorldTransformMatrixInverse;
}

FVector USceneComponent::GetWorldLocation() const
{
	return GetWorldTransformMatrix().GetLocation();
}

FQuaternion USceneComponent::GetWorldRotationAsQuaternion() const
{
	if (AttachParent && !IsUsingAbsoluteRotation())
	{
		// 쿼터니언 곱셈 q1*q2는 "q2 먼저, q1 나중"
		// 행벡터 행렬: World = Local * Parent (Local 먼저, Parent 나중)
		// 쿼터니언: World = Parent * Local (순서 반대!)
		return AttachParent->GetWorldRotationAsQuaternion() * RelativeRotation;
	}
	// bAbsoluteRotation=true 또는 부모 없음: Relative가 곧 World
	return RelativeRotation;
}

FVector USceneComponent::GetWorldRotation() const
{
	return GetWorldRotationAsQuaternion().ToEuler();
}

FVector USceneComponent::GetWorldScale3D() const
{
	return GetWorldTransformMatrix().GetScale();
}

void USceneComponent::SetWorldLocation(const FVector& NewLocation)
{
	if (AttachParent && !IsUsingAbsoluteLocation())
	{
		const FMatrix ParentWorldMatrixInverse = AttachParent->GetWorldTransformMatrixInverse();
		SetRelativeLocation(ParentWorldMatrixInverse.TransformPosition(NewLocation));
	}
	else
	{
		SetRelativeLocation(NewLocation);
	}
}

void USceneComponent::SetWorldRotation(const FVector& NewRotation)
{
	FQuaternion NewWorldRotationQuat = FQuaternion::FromEuler(NewRotation);
	if (AttachParent && !IsUsingAbsoluteRotation())
	{
		FQuaternion ParentWorldRotationQuat = AttachParent->GetWorldRotationAsQuaternion();
		// World = Parent * Local → Local = Parent^-1 * World
		SetRelativeRotation(ParentWorldRotationQuat.Inverse() * NewWorldRotationQuat);
	}
	else
	{
		SetRelativeRotation(NewWorldRotationQuat);
	}
}

void USceneComponent::SetWorldRotation(const FQuaternion& NewRotation)
{
	if (AttachParent && !IsUsingAbsoluteRotation())
	{
		FQuaternion ParentWorldRotationQuat = AttachParent->GetWorldRotationAsQuaternion();
		// World = Parent * Local → Local = Parent^-1 * World
		SetRelativeRotation(ParentWorldRotationQuat.Inverse() * NewRotation);
	}
	else
	{
		SetRelativeRotation(NewRotation);
	}
}

void USceneComponent::SetWorldScale3D(const FVector& NewScale)
{
	if (AttachParent && !IsUsingAbsoluteScale())
	{
		const FVector ParentWorldScale = AttachParent->GetWorldScale3D();
		SetRelativeScale3D(FVector(NewScale.X / ParentWorldScale.X, NewScale.Y / ParentWorldScale.Y, NewScale.Z / ParentWorldScale.Z));
	}
	else
	{
		SetRelativeScale3D(NewScale);
	}
}
