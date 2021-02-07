// All rights reserved.

#include "ActiveCrypteckBall.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "CharacterManager.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Crypteck.h"
#include "CryptecksLevel.h"
#include "Twalien.h"
#include "SharedLifeComponent.h"
#include "BasicWarrior.h"
#include "LifeComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
AActiveCrypteckBall::AActiveCrypteckBall() : DeltaTimeMultiplier(0.33f), Speed(2000), WidthSpace(350.f), HeightSpace(200), InitialAngle(0), CurrentOrbitTime(0),
DissolveFinished(false), Angle(0), Level(nullptr), HitDamage(10), DissolveCurrentTime(0), DissolveTotalTime(.75f), ICrypteckBall(), AActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("SpawnBallCollider"));
	Collider->SetSphereRadius(160);
	Collider->SetNotifyRigidBodyCollision(true);
	Collider->SetCollisionProfileName(FName("UnderGroundPhysicsActor"));
	RootComponent = Collider;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnBallMesh"));
	Mesh->SetCollisionProfileName(FName("NoCollision"));
	Mesh->SetupAttachment(RootComponent);

	Life = CreateDefaultSubobject<ULifeComponent>(TEXT("SpawnBallLife"));
	DeathDelegate.BindUFunction(this, FName("OnDeath"));
	Life->AddDelegateToDeath(DeathDelegate);
	AddOwnedComponent(Life);
}

// Called when the game starts or when spawned
void AActiveCrypteckBall::BeginPlay()
{
	Super::BeginPlay();

	UWorld * World = GetWorld();
	Level = Cast<ACryptecksLevel>(World->GetLevelScriptActor(World->GetCurrentLevel()));
}

void AActiveCrypteckBall::OnBallHit(const FHitResult & Hit)
{
	//Check if we hit players
	if (Hit.GetActor() && Hit.GetActor()->Tags.Contains("Player"))
	{
		ATwalien * Player = Cast<ATwalien>(Hit.GetActor());
		if (Player)
		{
			//Translate player
			FVector Direction = Player->GetActorLocation() - GetActorLocation();
			Direction.Z = 0;
			if (Direction == FVector::ZeroVector)
			{
				if (Level)
				{
					Direction = Level->GetLevelCenter() - GetActorLocation();
					Direction.Z = 0;
				}
			}
			Direction.Normalize();
			Player->SetActorLocation(Player->GetActorLocation() + (Direction * Collider->GetScaledSphereRadius() * 2.75f));

			//set new player HP
			Player->GetLifeComponent()->SetHitPoints(-HitDamage);
		}
	}
	else if (Hit.GetActor() && Hit.GetActor()->Tags.Contains("Enemy"))
	{
		ABasicWarrior * Ally = Cast<ABasicWarrior>(Hit.GetActor());
		if (Ally)
		{
			//Translate player
			FVector Direction = Ally->GetActorLocation() - GetActorLocation();
			Direction.Z = 0;
			if (Direction == FVector::ZeroVector)
			{
				if (Level)
				{
					Direction = Level->GetLevelCenter() - GetActorLocation();
					Direction.Z = 0;
				}
			}
			Direction.Normalize();
			Ally->SetActorLocation(Ally->GetActorLocation() + (Direction * Collider->GetScaledSphereRadius() * 2.75f));

			//set new player HP
			Ally->GetLifeComponent()->SetHitPoints(-HitDamage);
		}
	}
	else if (Hit.GetActor() && Hit.GetActor()->Tags.Contains("Crypteck's Sting"))
	{
		Hit.GetActor()->Destroy();
	}
	else if (Hit.GetActor() && Hit.GetActor()->GetName().Contains("ForceField"))
	{
		State = DISSOLVE_PRE_RETURN;
	}
}

void AActiveCrypteckBall::PrepareDissolveOperation()
{
	DissolveFinished = false;
	DissolveCurrentTime = 0;
}

void AActiveCrypteckBall::Dissolve(float DeltaSeconds)
{
	//Comment lines below when crypteck ball dissolve shader will be ready
	DissolveFinished = true;
	return;

	//------------------------------------------------------------------------

	const FName ParamName("Dissolve");

	DissolveCurrentTime += DeltaSeconds;
	if (DissolveCurrentTime < DissolveTotalTime)
	{
		float Percentage = DissolveCurrentTime / DissolveTotalTime;
		DissolveMaterial->SetScalarParameterValue(ParamName, Percentage);
	}
	else
	{
		DissolveMaterial->SetScalarParameterValue(ParamName, 1);
		DissolveFinished = true;
	}
}

void AActiveCrypteckBall::Undissolve(float DeltaSeconds)
{
	//Comment next line when crypteck ball dissolve shader will be ready
	DissolveFinished = true;
	return;

	//-----------------------------------------------------------------------

	const FName ParamName("Dissolve");

	DissolveCurrentTime += DeltaSeconds;
	if (DissolveCurrentTime < DissolveTotalTime)
	{
		float Percentage = DissolveCurrentTime / DissolveTotalTime;
		DissolveMaterial->SetScalarParameterValue(ParamName, 1 - Percentage);
	}
	else
	{
		DissolveMaterial->SetScalarParameterValue(ParamName, 0);
		DissolveFinished = true;
	}
}

void AActiveCrypteckBall::Orbit(float DeltaSeconds)
{
	FVector Location = OrbitCenter, NewPosition;
	FHitResult Hit;
	CurrentOrbitTime += DeltaTimeMultiplier * DeltaSeconds;
	Angle = InitialAngle + (CurrentOrbitTime * 360);
	CreateOrbitNewPosition(NewPosition);
	SetActorLocation(NewPosition, true, &Hit, ETeleportType::None);

	if (Hit.bBlockingHit && Hit.GetActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit %s with %s when orbiting."), *GetName(), *Hit.GetActor()->GetName());
	}
}

void AActiveCrypteckBall::Teleport(float DeltaSeconds)
{
	SetActorLocation(TeleportPosition);
}

void AActiveCrypteckBall::OnDeath()
{
	Destroy();
}

void AActiveCrypteckBall::ConvertAngleToOrbitTime(float NewAngle)
{
	CurrentOrbitTime = (NewAngle - InitialAngle) / 360;
}

// Called every frame
void AActiveCrypteckBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float NewAngle;

	switch (State)
	{
		case DISSOLVE_PRE_GOTO_ORBIT:
			Dissolve(DeltaTime);
			if (DissolveFinished)
			{
				State = TELEPORT_TO_ORBIT;
				DissolveFinished = false;
			}
		break;
		case TELEPORT_TO_ORBIT:
			NewAngle = Crypteck->AssignFreeAngle(this);
			ConvertAngleToOrbitTime(NewAngle);
			Angle = NewAngle;
			CreateOrbitNewPosition(TeleportPosition);
			Teleport(DeltaTime);
			if (GetActorLocation() == TeleportPosition)
			{
				State = IDLE;
				Crypteck->OnBallBeginOrbit(this);
				PrepareDissolveOperation();
			}
		break;
		case IDLE:
			Orbit(DeltaTime);
			Undissolve(DeltaTime);
		break;
		case DISSOLVE_PRE_ATTACK:
			Dissolve(DeltaTime);
			if (DissolveFinished)
			{
				State = TELEPORT_TO_ATTACK;
				DissolveFinished = false;
			}
		break;
		case TELEPORT_TO_ATTACK:
			Teleport(DeltaTime);
			if (GetActorLocation() == TeleportPosition)
			{
				State = DISSOLVE_POST_ATTACK;
				PrepareDissolveOperation();
			}
		break;
		case DISSOLVE_POST_ATTACK:
			Undissolve(DeltaTime);
			if (DissolveFinished)
			{
				State = ONTHEWAY;
				DissolveFinished = false;
			}
		break;
		case ONTHEWAY:
			Move(DeltaTime);
		break;
		case ONACTION:
			Action(DeltaTime);
		break;
		case DISSOLVE_PRE_RETURN:
			Dissolve(DeltaTime);
			if (DissolveFinished)
			{
				State = RETURNING;
				DissolveFinished = false;
			}
		break;
		case RETURNING:
			NewAngle = Crypteck->AssignFreeAngle(this);
			ConvertAngleToOrbitTime(NewAngle);
			Angle = NewAngle;
			CreateOrbitNewPosition(TeleportPosition);
			Teleport(DeltaTime);
			if (GetActorLocation() == TeleportPosition)
			{
				State = IDLE;
				Crypteck->OnBallBeginOrbit(this);
				PrepareDissolveOperation();
			}
		break;
		default:
		break;
	}
}

void AActiveCrypteckBall::SetTarget(const FVector & Target)
{	
	if (State == BALL_STATE::IDLE)
	{
		State = DISSOLVE_PRE_ATTACK;
		TargetPosition = Target;
		SetPositionToAttackTeleport();
		PrepareDissolveOperation();
	}
}

void AActiveCrypteckBall::Activate()
{
	if (State == DEACTIVATED)
	{
		State = DISSOLVE_PRE_GOTO_ORBIT;
		PrepareDissolveOperation();
	}
}

void AActiveCrypteckBall::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	Life->RemoveAllDelegatesFromDeath();
	Life->RemoveAllDelegatesFromDeath();
}

