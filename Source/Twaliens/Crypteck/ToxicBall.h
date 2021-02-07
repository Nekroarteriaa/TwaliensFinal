// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActiveCrypteckBall.h"
#include "ToxicBall.generated.h"

UCLASS()
class TWALIENS_API AToxicBall : public AActiveCrypteckBall
{
	GENERATED_BODY()
	
private:
	class AToxicArea * ConstructingArea;
	float Time;
	float MoveTime;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void CreateOrbitNewPosition(FVector & Position) override;

	virtual void Move(float DeltaSeconds) override;

	virtual void SetPositionToAttackTeleport() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ToxicBall")
	float ToxicReleaseTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ToxicBall")
	FVector FallDeviation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ToxicBall")
	float SinHeigth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ToxicBall")
	class APoolManager * ToxicAreaPool;

public:
	// Sets default values for this actor's properties
	AToxicBall();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Action(float DeltaSeconds) override;
};
