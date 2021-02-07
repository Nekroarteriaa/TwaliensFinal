// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrypteckBall.h"
#include "ActiveCrypteckBall.h"
#include "SmashBall.generated.h"

UCLASS()
class TWALIENS_API ASmashBall : public AActiveCrypteckBall
{
	GENERATED_BODY()

private:
	UFUNCTION()
	void ActionEnded();

	FTimerHandle ActionTimer;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void CreateOrbitNewPosition(FVector & Position) override;

	virtual void Move(float DeltaSeconds) override;

	virtual void SetPositionToAttackTeleport() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SmashBall")
	float RestTime;

public:	
	// Sets default values for this actor's properties
	ASmashBall();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Action(float DeltaSeconds) override;
};
