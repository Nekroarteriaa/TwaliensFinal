// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActiveCrypteckBall.h"
#include "FireBall.generated.h"

UCLASS()
class TWALIENS_API AFireBall : public AActiveCrypteckBall
{
	GENERATED_BODY()

private:
	class ABurningArea * ConstructingArea;
	float Time;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void CreateOrbitNewPosition(FVector & Position) override;

	virtual void Move(float DeltaSeconds) override;

	virtual void SetPositionToAttackTeleport() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireBall")
	float FireReleaseTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireBall")
	FVector FallDeviation;

public:
	// Sets default values for this actor's properties
	AFireBall();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Action(float DeltaSeconds) override;

};
