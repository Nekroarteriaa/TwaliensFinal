// All rights reserved.

#include "Crypteck.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "CharacterManager.h"
#include "SmashBall.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ConstructorHelpers.h"
#include "Engine/SkeletalMesh.h"
#include "ToxicBall.h"
#include "FireBall.h"
#include "SpawnBall.h"
#include "WeakObjectPtr.h"
#include "LifeComponent.h"
#include "CryptecksLevel.h"
#include "ActiveCrypteckBall.h"
#include "Sting.h"
#include "Kismet/KismetMathLibrary.h"
#include "BasicWarrior.h"
#include "Animation/AnimationAsset.h"

// Sets default values
ACrypteck::ACrypteck() : Phase(DEACTIVATED), SmashDead(false), ToxicDead(false), FireDead(false), SpawnDead(false), FallingTime(0), FallingToCenterSpeed(500), TimeForStingThrowing(3), SmashBallCD(5), ToxicBallCD(6),
FireBallCD(12), SpawnBallCD(15), DelayedToxicAction(false), DelayedFireAction(false), DelayedCheckTime(0.1f), Capsule(nullptr), Mesh(nullptr), Life(nullptr), StingPoolManager(nullptr), PlayerManager(nullptr), Smash(nullptr),
Toxic(nullptr), Spawn(nullptr), Fire(nullptr), Level(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Initializing Crypteck's Capsule
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CrypteckCollider"));
	Capsule->SetCapsuleHalfHeight(190.f);
	Capsule->SetCapsuleRadius(55.f);
	Capsule->SetCollisionProfileName(FName("UnderGroundPhysicsActor"));
	RootComponent = Capsule;

	//Initializing Crypteck's Mesh
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CrypteckMesh"));
	Mesh->SetRelativeLocation(FVector(0, 0, -195.f));
	Mesh->SetRelativeRotation(FRotator(0, 90, 0));
	Mesh->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
	Mesh->SetCollisionProfileName(FName("NoCollision"));
	ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Characters/Crypteck/SK_Crypteck_01"));
	if (MeshFinder.Succeeded())
	{
		Mesh->SetSkeletalMesh(MeshFinder.Object);
	}
	ConstructorHelpers::FObjectFinder<UAnimationAsset> MeshAnimFinder(TEXT("/Game/Characters/Crypteck/CrypteckIdle_Anim"));
	Mesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	if (MeshAnimFinder.Succeeded())
	{
		Mesh->OverrideAnimationData(MeshAnimFinder.Object);
	}
	Mesh->SetupAttachment(RootComponent);

	//Initializing Crypteck's wings
	Wings = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CrypteckWingsMesh"));
	Wings->SetRelativeLocation(FVector(0, 0, -195.f));
	Wings->SetRelativeRotation(FRotator(0, 90, 0));
	Wings->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
	Wings->SetCollisionProfileName(FName("NoCollision"));
	ConstructorHelpers::FObjectFinder<USkeletalMesh> WingsMeshFinder(TEXT("/Game/Characters/Crypteck/SK_CrypteckWings_01"));
	if (WingsMeshFinder.Succeeded())
	{
		Wings->SetSkeletalMesh(WingsMeshFinder.Object);
	}
	ConstructorHelpers::FObjectFinder<UAnimationAsset> WingsMeshAnimFinder(TEXT("/Game/Characters/Crypteck/CrypteckWingsIdle_Anim"));
	Wings->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	if (WingsMeshAnimFinder.Succeeded())
	{
		Wings->OverrideAnimationData(WingsMeshAnimFinder.Object);
	}
	Wings->SetupAttachment(RootComponent);

	//Initializing Crypteck's Life
	Life = CreateDefaultSubobject<ULifeComponent>(TEXT("CrypteckLife"));
	FScriptDelegate DeathDelegate;
	DeathDelegate.BindUFunction(this, FName("OnCrypteckDeath"));
	Life->AddDelegateToDeath(DeathDelegate);
	AddOwnedComponent(Life);

	//Initializing Crypteck's tag
	Tags.Add("Boss Crypteck");
}

// Called when the game starts or when spawned
void ACrypteck::BeginPlay()
{
	Super::BeginPlay();
	
	//Looking for Character Manager. TODO - Replace it with a call to Static class (Singleton Cache)
	TActorIterator<ACharacterManager> ManagerIterator(GetWorld());
	
	if (*ManagerIterator)
	{
		PlayerManager = *ManagerIterator;
	}

	//Getting Reference to Crypteck's level
	UWorld * World = GetWorld();
	ACryptecksLevel * MyLevel = Cast<ACryptecksLevel>(World->GetLevelScriptActor(World->GetCurrentLevel()));
	if (MyLevel)
	{
		Level = MyLevel;
	}
}

//************************************
// Method:    SetupBallLinks
// FullName:  ACrypteck::SetupBallLinks
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: AToxicBall * ToxicBall -> Reference to Toxic Ball
// Parameter: ASmashBall * SmashBall -> Reference to Smash Ball
// Parameter: AFireBall * FireBall -> Reference to Fire Ball
// Parameter: ASpawnBall * SpawnBall -> Reference to Spawn Ball
//************************************
void ACrypteck::SetupBallLinks(AToxicBall * ToxicBall, ASmashBall * SmashBall, AFireBall * FireBall, ASpawnBall * SpawnBall)
{
	//For each ball we do a setup, in this setup we ensure a correct operation
	//we initialize ball crypteck reference
	//Initial angle in its orbit, with this we avoid orbit hits
	//Delegate for all life component events

	//Setup for Smash ball
	Smash = SmashBall;
	Smash->SetCryptecksReference(this);
	Smash->SetInitialAngle(0);
	FScriptDelegate SmashUpdateDelegate, SmashDeathDelegate;
	SmashUpdateDelegate.BindUFunction(this, FName("OnSmashSphereGetDamage"));
	Smash->AddListenerToHPChanges(SmashUpdateDelegate);
	SmashDeathDelegate.BindUFunction(this, FName("OnSmashSphereDeath"));
	Smash->AddListenerToSphereDeath(SmashDeathDelegate);

	//Setup for Toxic ball
	Toxic = ToxicBall;
	Toxic->SetCryptecksReference(this);
	Toxic->SetInitialAngle(120);
	FScriptDelegate ToxicUpdateDelegate, ToxicDeathDelegate;
	ToxicUpdateDelegate.BindUFunction(this, FName("OnToxicSphereGetDamage"));
	Toxic->AddListenerToHPChanges(ToxicUpdateDelegate);
	ToxicDeathDelegate.BindUFunction(this, FName("OnToxicSphereDeath"));
	Toxic->AddListenerToSphereDeath(ToxicDeathDelegate);

	//Setup for Fire ball
	Fire = FireBall;
	Fire->SetCryptecksReference(this);
	Fire->SetInitialAngle(240);
	FScriptDelegate FireUpdateDelegate, FireDeathDelegate;
	FireUpdateDelegate.BindUFunction(this, FName("OnFireSphereGetDamage"));
	Fire->AddListenerToHPChanges(FireUpdateDelegate);
	FireDeathDelegate.BindUFunction(this, FName("OnFireSphereDeath"));
	Fire->AddListenerToSphereDeath(FireDeathDelegate);

	//Setup for Spawn ball
	Spawn = SpawnBall;
	FScriptDelegate SpawnUpdateDelegate, SpawnDeathDelegate;
	SpawnUpdateDelegate.BindUFunction(this, FName("OnSpawnSphereGetDamage"));
	Spawn->AddListenerToHPChanges(SpawnUpdateDelegate);
	SpawnDeathDelegate.BindUFunction(this, FName("OnSpawnSphereDeath"));
	Spawn->AddListenerToSphereDeath(SpawnDeathDelegate);
}

void ACrypteck::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	GetWorld()->GetTimerManager().ClearTimer(StingCDTimer);
	GetWorld()->GetTimerManager().ClearTimer(SmashBallTimer);
	GetWorld()->GetTimerManager().ClearTimer(ToxicBallTimer);
	GetWorld()->GetTimerManager().ClearTimer(SpawnBallTimer);
	GetWorld()->GetTimerManager().ClearTimer(FireBallTimer);
}

//************************************
// Method:    ThrowSting
// FullName:  ACrypteck::ThrowSting
// Access:    private 
// Returns:   void
// Qualifier:
//************************************
void ACrypteck::ThrowSting()
{
	if (PlayerManager)
	{
		const ATwalien * Player = PlayerManager->GetPlayerWithDustpan();
		FVector Origin, Offset, Destination;
		FActorSpawnParameters Parameters;
		
		//Destination
		Destination = Player->GetActorLocation();
		Destination.Z -= Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - 25;

		//Position
		Origin = GetActorLocation();
		Origin.Z -= Capsule->GetScaledCapsuleHalfHeight();
		Offset = Destination - Origin;
		Offset.Normalize();
		Origin += Offset * Capsule->GetScaledCapsuleRadius() * 2;

		//Rotation
		const FRotator NewRotator = UKismetMathLibrary::FindLookAtRotation(Origin, Destination);

		ASting * Sting = nullptr;
		if (StingPoolManager)
		{
			Sting = Cast<ASting>(StingPoolManager->GetActorFromPool());
			Sting->SetActorLocation(Origin);
			Sting->AddActorWorldRotation(NewRotator);
		}
		else
		{
			Sting = GetWorld()->SpawnActor<ASting>(Origin, NewRotator, Parameters);
		}
		Sting->Init(Offset);
	}
}

//************************************
// Method:    OnBallBeginOrbit
// FullName:  ACrypteck::OnBallBeginOrbit
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: AActiveCrypteckBall * Ball -> Ball that  begins its orbit
//************************************
void ACrypteck::OnBallBeginOrbit(AActiveCrypteckBall * Ball)
{
	if (Ball == Smash)
	{
		GetWorld()->GetTimerManager().SetTimer(SmashBallTimer, this, &ACrypteck::SetTargetToSmashBall, 1, false, SmashBallCD);
	}
	else if (Ball == Toxic)
	{
		GetWorld()->GetTimerManager().SetTimer(ToxicBallTimer, this, &ACrypteck::SetTargetToToxicBall, 1, false, ToxicBallCD);
	}
	else if (Ball == Fire)
	{
		GetWorld()->GetTimerManager().SetTimer(FireBallTimer, this, &ACrypteck::SetTargetToFireBall, 1, false, FireBallCD);
	}
}

void ACrypteck::SetTargetToSmashBall()
{
	if (PlayerManager && Smash)
	{
		SmashTarget = PlayerManager->GetPlayerWithWeapon()->GetActorLocation();
		//check if target location is conflict free
		if (IsThereLocationConflict(SmashTarget, Smash->GetSphereRadius()))
		{
			//if is not conflict free we wait DelayedChecktime
			GetWorld()->GetTimerManager().SetTimer(SmashBallTimer, this, &ACrypteck::SetTargetToSmashBall, 1, false, DelayedCheckTime);
		}
		//no conflict
		else
		{
			//set target
			Smash->SetTarget(PlayerManager->GetPlayerWithWeapon()->GetActorLocation());
		}
	}
}

void ACrypteck::SetTargetToToxicBall()
{
	if (Level && Toxic)
	{
		FVector TargetPosition;
		if (!DelayedToxicAction && Level->QueryInnerFreeCell(TargetPosition))
		{
			//check if target location is conflict free
			if (IsThereLocationConflict(TargetPosition, Toxic->GetSphereRadius()))
			{
				//if is not conflict free we wait DelayedChecktime
				GetWorld()->GetTimerManager().SetTimer(ToxicBallTimer, this, &ACrypteck::SetTargetToToxicBall, 1, false, DelayedCheckTime);
				//set we delayed action flags
				DelayedToxicAction = true;
				ToxicTarget = TargetPosition;
			}
			else
			{
				Toxic->SetTarget(TargetPosition);
			}
		}
		else if (DelayedToxicAction)
		{
			//check if target location is conflict free
			if (IsThereLocationConflict(ToxicTarget, Toxic->GetSphereRadius()))
			{
				//if is not conflict free we wait DelayedChecktime
				GetWorld()->GetTimerManager().SetTimer(ToxicBallTimer, this, &ACrypteck::SetTargetToToxicBall, 1, false, DelayedCheckTime);
			}
			else
			{
				Toxic->SetTarget(ToxicTarget);
				DelayedToxicAction = false;
			}
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(ToxicBallTimer, this, &ACrypteck::SetTargetToToxicBall, 1, false, ToxicBallCD);
		}
	}
}

void ACrypteck::SetTargetToFireBall()
{
	if (Level && Fire)
	{
		FVector TargetPosition;
		if (!DelayedFireAction && Level->QueryOuterFreeCell(TargetPosition))
		{
			//check if target location is conflict free
			if (IsThereLocationConflict(TargetPosition, Fire->GetSphereRadius()))
			{
				//if is not conflict free we wait DelayedChecktime
				GetWorld()->GetTimerManager().SetTimer(FireBallTimer, this, &ACrypteck::SetTargetToFireBall, 1, false, DelayedCheckTime);
				//set delayed action flags
				DelayedFireAction = true;
				FireTarget = TargetPosition;
			}
			else
			{
				Fire->SetTarget(TargetPosition);
			}
		}
		else if (DelayedFireAction)
		{
			//check if target location is conflict free
			if (IsThereLocationConflict(FireTarget, Fire->GetSphereRadius()))
			{
				//if is not conflict free we wait DelayedChecktime
				GetWorld()->GetTimerManager().SetTimer(FireBallTimer, this, &ACrypteck::SetTargetToFireBall, 1, false, DelayedCheckTime);
			}
			else
			{
				Fire->SetTarget(FireTarget);
				DelayedFireAction = false;
			}
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(FireBallTimer, this, &ACrypteck::SetTargetToFireBall, 1, false, FireBallCD);
		}
	}
}

void ACrypteck::DoSpawnBallEffect()
{
	if (Spawn)
	{
		Spawn->CallReinforcements();
	}
}

bool ACrypteck::IsThereLocationConflict(const FVector & NewLocation, float Radius) const
{
	bool Conflict = false;

	Conflict |= Fire && Fire->IsDoingItsAction() && FVector::Dist2D(FireTarget, NewLocation) < Radius + Fire->GetSphereRadius();
	Conflict |= Toxic && Toxic->IsDoingItsAction() && FVector::Dist2D(ToxicTarget, NewLocation) < Radius + Toxic->GetSphereRadius();
	Conflict |= Smash && Smash->IsDoingItsAction() && FVector::Dist2D(SmashTarget, NewLocation) < Radius + Smash->GetSphereRadius();
	Conflict |= Spawn && FVector::Dist2D(Spawn->GetActorLocation(), NewLocation) < Radius + Spawn->GetSphereRadius();

	return Conflict;
}

void ACrypteck::OnSmashSphereGetDamage()
{
	TriggerPhaseChange(GetSphereLifeMinimumPercentage(Smash->GetSphereLifeComponent()));
}

void ACrypteck::OnFireSphereGetDamage()
{
	TriggerPhaseChange(GetSphereLifeMinimumPercentage(Fire->GetSphereLifeComponent()));
}

void ACrypteck::OnToxicSphereGetDamage()
{
	TriggerPhaseChange(GetSphereLifeMinimumPercentage(Toxic->GetSphereLifeComponent()));
}

void ACrypteck::OnSpawnSphereGetDamage()
{
	TriggerPhaseChange(GetSphereLifeMinimumPercentage(Spawn->GetSphereLifeComponent()));
}

void ACrypteck::OnSmashSphereDeath()
{
	SmashDead = true;
	Smash = nullptr;
	TriggerPhaseChange(1.f);
	GetWorld()->GetTimerManager().ClearTimer(SmashBallTimer);
}

void ACrypteck::OnFireSphereDeath()
{
	FireDead = true;
	Fire = nullptr;
	TriggerPhaseChange(1.f);
	GetWorld()->GetTimerManager().ClearTimer(FireBallTimer);
}

void ACrypteck::OnToxicSphereDeath()
{
	ToxicDead = true;
	Toxic = nullptr;
	TriggerPhaseChange(1.f);
	GetWorld()->GetTimerManager().ClearTimer(ToxicBallTimer);
}

void ACrypteck::OnSpawnSphereDeath()
{
	SpawnDead = true;
	Spawn = nullptr;
	TriggerPhaseChange(1.f);
	GetWorld()->GetTimerManager().ClearTimer(SpawnBallTimer);
	
	for (TActorIterator<ABasicWarrior> BasicWarriorControllerIterator(GetWorld()); BasicWarriorControllerIterator; ++BasicWarriorControllerIterator)
	{
		ABasicWarrior * Warrior = *BasicWarriorControllerIterator;

		if (Warrior)
		{
			Warrior->Disconnect();
		}
	}
}

void ACrypteck::OnCrypteckDeath()
{
	Destroy();
}

float ACrypteck::GetSphereLifeMinimumPercentage(const ULifeComponent * SphereLife)
{
	return SphereLife->GetHitPoints() / SphereLife->GetMaxHitPoints();
}

void ACrypteck::TriggerPhaseChange(float NewLocalMinimumPercentageSphereHP)
{
	switch (Phase)
	{
		case PHASE_BALLSMASHTOXIC:
			if (NewLocalMinimumPercentageSphereHP <= 0.66f && Spawn)
			{
				Phase = PHASE_BALLSPAWN;
				Spawn->Activate();
				GetWorld()->GetTimerManager().SetTimer(SpawnBallTimer, this, &ACrypteck::DoSpawnBallEffect, SpawnBallCD, true, 0);
			}
		break;
		case PHASE_BALLSPAWN:
			if (NewLocalMinimumPercentageSphereHP <= 0.33f && Fire)
			{
				Phase = PHASE_BALLFIRE;
				Fire->Activate();
				Fire->SetOrbitCenter(GetActorLocation());
			}
		break;
		case PHASE_BALLFIRE:
			if (SmashDead && ToxicDead && SpawnDead && FireDead)
			{
				FVector DestinationPosition;
				if (Level)
				{
					Phase = PHASE_REVENGE;

					GetWorld()->GetTimerManager().ClearTimer(StingCDTimer);

					OriginPosition = GetActorLocation();
					FallingDestination = Level->GetLevelCenter();
					FallingTime = 0;
					FallingTimeMultiplier = FallingToCenterSpeed / FVector::Dist(OriginPosition, FallingDestination);
				}
			}
		break;
		default:
		break;
	}
}

// Called every frame
void ACrypteck::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Phase != DEACTIVATED)
	{
		if (Smash && Smash->GetOrbitCenter() != GetActorLocation())
		{
			Smash->SetOrbitCenter(GetActorLocation());
		}

		if (Toxic && Toxic->GetOrbitCenter() != GetActorLocation())
		{
			Toxic->SetOrbitCenter(GetActorLocation());
		}

		if (Fire && Fire->GetOrbitCenter() != GetActorLocation())
		{
			Fire->SetOrbitCenter(GetActorLocation());
		}
	}

	if (Phase == PHASE_REVENGE && FallingTime <= 1.f)
	{
		FallingTime += DeltaTime * FallingTimeMultiplier;

		SetActorLocation(FMath::Lerp(OriginPosition, FallingDestination, FallingTime), true, nullptr);
	}
}

void ACrypteck::Activate()
{
	if (Phase == DEACTIVATED && Smash && Toxic)
	{
		Phase = PHASE_BALLSMASHTOXIC;
		Smash->Activate();
		Smash->SetOrbitCenter(GetActorLocation());
		Toxic->Activate();
		Toxic->SetOrbitCenter(GetActorLocation());
		GetWorld()->GetTimerManager().SetTimer(StingCDTimer, this, &ACrypteck::ThrowSting, TimeForStingThrowing, true, 0);
		if (DemoVersion)
		{
			TriggerPhaseChange(0.6f);
			TriggerPhaseChange(0.3f);
		}
	}
}

float ACrypteck::GetFreeAngle(class AActiveCrypteckBall * NextBall, class AActiveCrypteckBall * PreviousBall, class AActiveCrypteckBall * SelfBall)
{
	if (NextBall->GetBallIsInOrbit())
	{
		return NextBall->GetCurrentOrbitAngle() - 120;
	}
	else if (PreviousBall->GetBallIsInOrbit())
	{
		return PreviousBall->GetCurrentOrbitAngle() + 120;
	}
	else
	{
		return SelfBall->GetCurrentOrbitAngle();
	}
}

float ACrypteck::AssignFreeAngle(AActiveCrypteckBall * MyBall)
{
	if (MyBall == Smash)
	{
		//Other two balls are alive
		if (!ToxicDead && !FireDead)
		{
			return GetFreeAngle(Toxic, Fire, Smash);
		}
		//Only one ball is alive
		else if (!FireDead && Fire->GetBallIsInOrbit())
		{
			return Fire->GetCurrentOrbitAngle() + 120;
		}
		else if (!ToxicDead && Toxic->GetBallIsInOrbit())
		{
			return Toxic->GetCurrentOrbitAngle() - 120;
		}
		//No one are alive
		else
		{
			return MyBall->GetCurrentOrbitAngle();
		}
	}
	else if (MyBall == Toxic)
	{
		//Other two balls are alive
		if (!FireDead && !SmashDead)
		{
			return GetFreeAngle(Fire, Smash, Toxic);
		}
		//Only one ball is alive
		else if (!SmashDead && Smash->GetBallIsInOrbit())
		{
			return Smash->GetCurrentOrbitAngle() + 120;
		}
		else if (!FireDead && Fire->GetBallIsInOrbit())
		{
			return Fire->GetCurrentOrbitAngle() - 120;
		}
		//No one are alive
		else
		{
			return MyBall->GetCurrentOrbitAngle();
		}
	}
	else if (MyBall == Fire)
	{
		//Other two balls are alive
		if (!ToxicDead && !SmashDead)
		{
			return GetFreeAngle(Smash, Toxic, Fire);
		}
		//Only one ball is alive
		else if (!ToxicDead && Toxic->GetBallIsInOrbit())
		{
			return Toxic->GetCurrentOrbitAngle() + 120;
		}
		else if (!SmashDead && Smash->GetBallIsInOrbit())
		{
			return Smash->GetCurrentOrbitAngle() - 120;
		}
		//No one are alive
		else
		{
			return MyBall->GetCurrentOrbitAngle();
		}
	}
	else
		return 0;
}
