#pragma once
#include "Widget.h"

class UMaterial;
class UTargetActorTransformWidget : public UWidget
{
	DECLARE_CLASS(UTargetActorTransformWidget, UWidget)
public:
	void Initialize() override;
	void Update() override;
	void RenderWidget() override;
	void PostProcess() override;

	void UpdateTransformFromActor();
	void UpdateMaterialListCash();
	void UpdateStaticMeshListCash();
	void ApplyTransformToActor() const;
	void RenderComponents();
	void RenderStatitMeshComponent(UStaticMeshComponent* Component);
	void RenderMaterials(UStaticMeshComponent* Component);

	// Special Member Function
	UTargetActorTransformWidget();
	~UTargetActorTransformWidget() override;

private:
	AActor* SelectedActor;

	//캐싱용
	TArray<UStaticMesh*> StaticMeshList;
	TArray<UMaterial*> MaterialList;
	bool bMaterialListDirty = true;
	bool bStaticMeshListDirty = true;

	FVector EditLocation;
	FVector EditRotation;
	FVector EditScale;
	bool bScaleChanged;
	bool bRotationChanged;
	bool bPositionChanged;
	uint64 LevelMemoryByte;
	uint32 LevelObjectCount;

	ULevel* LastLevel = nullptr;
};
