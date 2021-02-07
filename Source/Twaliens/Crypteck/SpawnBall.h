// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrypteckBall.h"
#include "LifeComponent.h"
#include "Components/SphereComponent.h"
#include "SpawnBall.generated.h"

UCLASS()
class TWALIENS_API ASpawnBall : public AActor, public ICrypteckBall
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnBall();

private:
	bool EnemiesAwaken;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ActiveCrypteckBall")
	class UStaticMeshComponent * Mesh;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ActiveCrypteckBall")
	class USphereComponent * Collider;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ActiveCrypteckBall")
	class ULifeComponent * Life;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnBall")
	TArray<class AEnemySpawner *> SpawnersLinkedToSpawnBall;

	UFUNCTION()
	void OnDeath();
	FScriptDelegate DeathDelegate;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Action(float DeltaSeconds) override;

	virtual void Activate() override;

	void CallReinforcements();

	FORCEINLINE ULifeComponent * GetSphereLifeComponent() { return Life; }
	FORCEINLINE float GetSphereRadius() const { return Collider->GetScaledSphereRadius(); }
	FORCEINLINE void AddListenerToHPChanges(const FScriptDelegate & Delegate) { Life->AddDelegateToHPUpdate(Delegate); }
	FORCEINLINE void AddListenerToSphereDeath(const FScriptDelegate & Delegate) { Life->AddDelegateToDeath(Delegate); }
};
