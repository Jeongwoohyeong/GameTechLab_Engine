#pragma once
#include "Actor/Public/Actor.h"
#include "Global/WeakObjectPtr.h"

class APlayerController;

/**
 * @brief Pawn is the base class for all actors that can be possessed by players or AI.
 * It provides the basic framework for being controlled by a Controller.
 */
UCLASS()
class APawn : public AActor
{
	GENERATED_BODY()
	DECLARE_CLASS(APawn, AActor)

public:
	APawn();
	virtual ~APawn();

	// Lifecycle
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Controller Management
	virtual void PossessedBy(APlayerController* NewController);
	virtual void UnPossessed();

	APlayerController* GetController() const;
	bool IsControlled() const { return Controller.IsValid(); }

protected:
	// The controller currently possessing this pawn
	TWeakObjectPtr<APlayerController> Controller;
};
