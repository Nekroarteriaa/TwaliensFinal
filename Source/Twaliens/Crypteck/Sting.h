// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SuckableObjects.h"
#include "Interfaces/IPooledObject.h"
#include "Sting.generated.h"

UCLASS()
class TWALIENS_API ASting : public AActor, public ISuckableObjects, public IIPooledObject
{
	GENERATED_BODY()

private:
	bool ActorIsCrypteckBall(const TArray<FName> & Tags);
	void HitPlayer(const class ATwalien & Player);
	void HitCrypteckBall(const class AActor & Ball);
	void HitCrypteck(const class AActor & Boss);
	void HitEnvironment(const FVector & ImpactNormal);
	void HitEnemy(const class AActor & Enemy);
	void HitForceField();
	void HitSting(ASting & OtherSting);

	FTimerHandle LifeTimer;
	float LifeTime;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sting")
	class UStaticMeshComponent * Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sting")
	class UBoxComponent * Collider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crypteck")
	float Speed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crypteck")
	float Rotation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crypteck")
	float DamageToPlayer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crypteck")
	float DamageToBall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crypteck")
	float DamageToEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crypteck")
	float ReboundForce;
	
	bool Pickable, Hittable;

	UFUNCTION()
	void OnStingHit(class UPrimitiveComponent * HitComponent, AActor * OtherActor, class UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit);

public:	
	// Sets default values for this actor's properties
	ASting();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Init(const FVector & NewDirection, bool CrypteckAmmunition = true);

	virtual void OnBeingSucked(const class UDustpan & Dustpan) override;

	virtual void OnObjectExitsPool() override;

	virtual void OnObjectEnterPool() override;

	FORCEINLINE bool CanBeSucked() { return Pickable; }
};
