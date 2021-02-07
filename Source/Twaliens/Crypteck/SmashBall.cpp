// All rights reserved.

#include "SmashBall.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "ConstructorHelpers.h"
#include "LifeComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "CharacterManager.h"
#include "Twalien.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CryptecksLevel.h"
#include "SharedLifeComponent.h"
#include "BasicWarrior.h"

// Sets default values
ASmashBall::ASmashBall() : RestTime(3.f), AActiveCrypteckBall()
{
	//Super::AActiveCrypteckBall();
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Tags.Add(FName("Crypteck's ball: Smash"));

	ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/EditorMeshes/ArcadeEditorSphere.ArcadeEditorSphere"));
	ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Game/Materials/M_Blue.M_Blue"));
	if (MeshFinder.Succeeded())
	{
		Mesh->SetStaticMesh(MeshFinder.Object);
	}
	if (MaterialFinder.Succeeded())
	{
		Mesh->SetMaterial(0, MaterialFinder.Object);
	}
}

// Called when the game starts or when spawned
void ASmashBall::BeginPlay()
{
	Super::BeginPlay();
}

void ASmashBall::CreateOrbitNewPosition(FVector & Position)
{
	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, FMath::DegreesToRadians(Angle));
	Position.Set(OrbitCenter.X + (Cos * WidthSpace), OrbitCenter.Y + (Sin * WidthSpace), OrbitCenter.Z);
}

void ASmashBall::Move(float DeltaSeconds)
{
	FHitResult Hit;
	FVector Displacement = TargetPosition - GetActorLocation();
	FVector Movement = DisplacementNormal * Speed * DeltaSeconds;
	
	if (Movement.SizeSquared() >= Displacement.SizeSquared())
	{
		Movement = TargetPosition;
		State = ONACTION;
	}
	else
	{
		Movement += GetActorLocation();
	}
	
	SetActorLocation(Movement, true, &Hit, ETeleportType::None);
	
	if (Hit.bBlockingHit && Hit.GetActor())
	{
		OnBallHit(Hit);
	}
}

void ASmashBall::SetPositionToAttackTeleport()
{
	TeleportPosition = TargetPosition + (FVector(0, 0, 1000));
	DisplacementNormal = FVector(0, 0, -1);
}

void ASmashBall::ActionEnded()
{
	State = DISSOLVE_PRE_RETURN;
	PrepareDissolveOperation();
}

// Called every frame
void ASmashBall::Tick(float DeltaTime)
{
	//Al logic is in parent's tick
	Super::Tick(DeltaTime);
}

void ASmashBall::Action(float DeltaSeconds)
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(ActionTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(ActionTimer, this, &ASmashBall::ActionEnded, 1, false, RestTime);
	}
}