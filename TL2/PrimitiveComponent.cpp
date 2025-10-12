#include "pch.h"
#include "PrimitiveComponent.h"
#include "SceneLoader.h"
#include "SceneComponent.h"
#include "SceneRotationUtils.h"

void UPrimitiveComponent::SetMaterial(const FString& FilePath)
{
    Material = UResourceManager::GetInstance().Load<UMaterial>(FilePath);
}

void UPrimitiveComponent::TickComponent(float DeltaSeconds)
{
}

UObject* UPrimitiveComponent::Duplicate()
{
    UPrimitiveComponent* DuplicatedComponent = NewObject<UPrimitiveComponent>(*this);

    // Transform 속성 복사 (부모 속성)
    DuplicatedComponent->RelativeLocation = this->RelativeLocation;
    DuplicatedComponent->RelativeRotation = this->RelativeRotation;
    DuplicatedComponent->RelativeScale = this->RelativeScale;
    DuplicatedComponent->UpdateRelativeTransform();

    // PrimitiveComponent 속성 복사
    DuplicatedComponent->Material = this->Material;

    DuplicatedComponent->DuplicateSubObjects();

    return DuplicatedComponent;
}

void UPrimitiveComponent::DuplicateSubObjects()
{
    Super_t::DuplicateSubObjects();


}
