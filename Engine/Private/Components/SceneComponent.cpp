#include "pch.h"
#include "Components/SceneComponent.h"
#include "Manager/ResourceManager.h"

#include <algorithm>

IMPLEMENT_CLASS(USceneComponent, UActorComponent)

USceneComponent::USceneComponent()
{
	ComponentType = EComponentType::Scene;
}

void USceneComponent::SetParentAttachment(USceneComponent* NewParent)
{
	if (NewParent == this)
	{
		return;
	}

	if (NewParent == ParentAttachment)
	{
		return;
	}

	//부모의 조상중에 내 자식이 있으면 순환참조 -> 스택오버플로우 일어남.
	for (USceneComponent* Ancester = NewParent; Ancester; Ancester = NewParent->ParentAttachment)
	{
		if (NewParent == this) //조상중에 내 자식이 있다면 조상중에 내가 있을 것임.
			return;
	}

	//부모가 될 자격이 있음, 이제 부모를 바꿈.

	if (ParentAttachment) //부모 있었으면 이제 그 부모의 자식이 아님
	{
		ParentAttachment->RemoveChild(this);
	}

	ParentAttachment = NewParent;

	NewParent->Children.push_back(this);

	MarkAsDirty();

}

void USceneComponent::RemoveChild(USceneComponent* ChildDeleted)
{
	Children.erase(std::remove(Children.begin(), Children.end(), this), Children.end());
}

void USceneComponent::MarkAsDirty()
{
	bIsTransformDirty = true;
	bIsTransformDirtyInverse = true;

	for (USceneComponent* Child : Children)
	{
		Child->MarkAsDirty();
	}
}

void USceneComponent::SetRelativeLocation(const FVector& Location)
{
	RelativeLocation = Location;
	MarkAsDirty();
}

void USceneComponent::SetRelativeRotation(const FVector& Rotation)
{
    RelativeRotation = Rotation;
    // Keep quaternion in sync with UI degrees
    RelativeRotationQuat = FQuat::FromEulerXYZ(RelativeRotation);
    MarkAsDirty();
}
void USceneComponent::SetRelativeScale3D(const FVector& Scale)
{
	FVector ActualScale = Scale;
	ActualScale.X = std::max(ActualScale.X, MinScale);
	ActualScale.Y = std::max(ActualScale.Y, MinScale);
	ActualScale.Z = std::max(ActualScale.Z, MinScale);
	RelativeScale3D = ActualScale;
	MarkAsDirty();
}

void USceneComponent::SetUniformScale(bool bIsUniform)
{
	bIsUniformScale = bIsUniform;
}

bool USceneComponent::IsUniformScale() const
{
	return bIsUniformScale;
}

const FVector& USceneComponent::GetRelativeLocation() const
{
	return RelativeLocation;
}
const FVector& USceneComponent::GetRelativeRotation() const
{
	return RelativeRotation;
}
const FVector& USceneComponent::GetRelativeScale3D() const
{
	return RelativeScale3D;
}

const FVector& USceneComponent::GetWorldLocation() const
{
	const FMatrix& WorldMatrix = GetWorldTransformMatrix();
	return FVector(WorldMatrix.Data[3][0], WorldMatrix.Data[3][1], WorldMatrix.Data[3][2]);
}

const FMatrix& USceneComponent::GetWorldTransformMatrix() const
{
    if (bIsTransformDirty)
    {
        // Quaternion-based TRS (row-major): I * S * R * T
        WorldTransformMatrix = FMatrix::GetModelMatrix(RelativeLocation, RelativeRotationQuat, RelativeScale3D);

        for (USceneComponent* Ancester = ParentAttachment; Ancester; Ancester = Ancester->ParentAttachment)
        {
            WorldTransformMatrix *= FMatrix::GetModelMatrix(Ancester->RelativeLocation, Ancester->RelativeRotationQuat, Ancester->RelativeScale3D);
        }

        bIsTransformDirty = false;
    }

	return WorldTransformMatrix;
}

const FMatrix& USceneComponent::GetWorldTransformMatrixInverse() const
{

    if (bIsTransformDirtyInverse)
    {
        WorldTransformMatrixInverse = FMatrix::Identity;
        for (USceneComponent* Ancestor = ParentAttachment; Ancestor; Ancestor = Ancestor->ParentAttachment)
        {
            WorldTransformMatrixInverse = FMatrix::GetModelMatrixInverse(Ancestor->RelativeLocation, Ancestor->RelativeRotationQuat, Ancestor->RelativeScale3D) * WorldTransformMatrixInverse;
        }
        WorldTransformMatrixInverse = WorldTransformMatrixInverse * FMatrix::GetModelMatrixInverse(RelativeLocation, RelativeRotationQuat, RelativeScale3D);

        bIsTransformDirtyInverse = false;
    }

	return WorldTransformMatrixInverse;
}


















