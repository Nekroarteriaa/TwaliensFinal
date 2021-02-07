// All rights reserved.

#include "Sting.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/PrimitiveComponent.h"
#include "Twalien.h"
#include "ActiveCrypteckBall.h"
#include "LifeComponent.h"
#include "SharedLifeComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Crypteck.h"
#include "Dustpan.h"
#include "CharacterManager.h"
#include "BasicWarrior.h"

// Sets default values
ASting::ASting() : Speed(2500), Rotation(450), DamageToPlayer(5), DamageToBall(8), DamageToEnemies(33), Pickable(false), ReboundForce(500000.f), LifeTime(12.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("StingCollider"));
	Collider->SetBoxExtent(FVector(15.215841f, 50.286522f, 49.960678f));
	Collider->SetGenerateOverlapEvents(true);
	Collider->SetNotifyRigidBodyCollision(true);
	Collider->SetEnableGravity(false);
	Collider->SetSimulatePhysics(true);
	Collider->SetCollisionProfileName(FName("Custom..."));
	Collider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Collider->SetCollisionObjectType(ECC_WorldDynamic);
	Collider->SetCollisionResponseToAllChannels(ECR_Block);
	Collider->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
	Collider->SetCollisionResponseToChannel(ECC_Camera, ECR_Overlap);
	Collider->OnComponentHit.AddDynamic(this, &ASting::OnStingHit);
	Collider->SetUseCCD(true);
	Collider->SetRelativeRotation(FRotator(270, 0, 0));
	RootComponent = Collider;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StingMesh"));
	Mesh->SetCollisionProfileName(FName("NoCollision"));
	ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BufferVisualization/FinalImage"));
	if (MeshFinder.Succeeded())
	{
		Mesh->SetStaticMesh(MeshFinder.Object);
	}
	if (MaterialFinder.Succeeded())
	{
		Mesh->SetMaterial(0, MaterialFinder.Object);
	}
	Mesh->SetRelativeScale3D(FVector(.3f, 1, 1));
	Mesh->SetupAttachment(RootComponent);

	Tags.Add("Crypteck's Sting");

}

// Called when the game starts or when spawned
void ASting::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASting::OnStingHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	ATwalien * Player = Cast<ATwalien>(OtherActor);
	
	if (Player && !Pickable)
	{
		HitPlayer(*Player);
	}
	else if (OtherActor && ActorIsCrypteckBall(OtherActor->Tags))
	{
		if (Hittable)
		{
			HitCrypteckBall(*OtherActor);
		}
		else
		{
			HitEnvironment(Hit.ImpactNormal);
		}
	}
	else if (OtherActor && OtherActor->Tags.Contains("Boss Crypteck") && Hittable)
	{
		HitCrypteck(*OtherActor);
	}
	else if (OtherActor && OtherActor->Tags.Contains("Environment"))
	{
		HitEnvironment(Hit.ImpactNormal);
	}
	else if (OtherActor && OtherActor->Tags.Contains("Enemy"))
	{
		if (Hittable)
		{
			HitEnemy(*OtherActor);
		}
		else
		{
			HitEnvironment(Hit.ImpactNormal);
		}
	}
	else if (OtherActor && OtherActor->GetName().Contains("ForceField"))
	{
		HitForceField();
	}
	else if (OtherActor && OtherActor->Tags.Contains("Crypteck's Sting"))
	{
		HitSting(*Cast<ASting>(OtherActor));
	}
}

bool ASting::ActorIsCrypteckBall(const TArray<FName> & ActorTags)
{
	bool found = false;
	for (const FName Name : ActorTags)
	{
		found = Name.ToString().Contains("Crypteck's ball");
		if (found)
			return found;
	}
	return found;
}

void ASting::HitPlayer(const class ATwalien & Player)
{
	Player.GetLifeComponent()->SetHitPoints(-DamageToPlayer);
	GetWorld()->GetTimerManager().ClearTimer(LifeTimer);
	
	if (HasValidPool())
	{
		ReturnToPool();
	}
	else
	{
		Destroy();
	}
}

void ASting::HitCrypteckBall(const class AActor & Ball)
{
	ULifeComponent * Life = Cast<ULifeComponent>(Ball.GetComponentByClass(ULifeComponent::StaticClass()));
	if (Life)
	{
		Life->SetHitPoints(-DamageToBall);
	}

	GetWorld()->GetTimerManager().ClearTimer(LifeTimer);

	if (HasValidPool())
	{
		ReturnToPool();
	}
	else
	{
		Destroy();
	}
}

void ASting::HitCrypteck(const class AActor & Boss)
{
	ULifeComponent * Life = Cast<ULifeComponent>(Boss.GetComponentByClass(ULifeComponent::StaticClass()));
	if (Life)
	{
		Life->SetHitPoints(-DamageToBall);
	}

	GetWorld()->GetTimerManager().ClearTimer(LifeTimer);

	if (HasValidPool())
	{
		ReturnToPool();
	}
	else
	{
		Destroy();
	}
}

void ASting::HitEnvironment(const FVector & ImpactNormal)
{
	if (!Pickable)
	{
		Collider->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Collider->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		Collider->SetEnableGravity(true);

		Collider->AddForce(ImpactNormal * ReboundForce);
		Pickable = true;

		if (!GetWorld()->GetTimerManager().IsTimerActive(LifeTimer) && HasValidPool())
		{
			GetWorld()->GetTimerManager().SetTimer(LifeTimer, this, &ASting::ReturnToPool, 1, false, LifeTime);
		}
	}
}

void ASting::HitEnemy(const class AActor & Enemy)
{
	ULifeComponent * Life = Cast<ULifeComponent>(Enemy.GetComponentByClass(ULifeComponent::StaticClass()));
	if (Life)
	{
		Life->SetHitPoints(-DamageToEnemies);
	}

	GetWorld()->GetTimerManager().ClearTimer(LifeTimer);

	if (HasValidPool())
	{
		ReturnToPool();
	}
	else
	{
		Destroy();
	}
}

void ASting::HitForceField()
{
	GetWorld()->GetTimerManager().ClearTimer(LifeTimer);

	if (HasValidPool())
	{
		ReturnToPool();
	}
	else
	{
		Destroy();
	}
}

void ASting::HitSting(ASting & OtherSting)
{
	GetWorld()->GetTimerManager().ClearTimer(LifeTimer);
	GetWorld()->GetTimerManager().ClearTimer(OtherSting.LifeTimer);

	if (OtherSting.HasValidPool())
	{
		OtherSting.ReturnToPool();
	}
	else
	{
		OtherSting.Destroy();
	}


	if (HasValidPool())
	{
		ReturnToPool();
	}
	else
	{
		Destroy();
	}
}

// Called every frame
void ASting::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASting::Init(const FVector & NewDirection, bool CrypteckAmmunition)
{
	if (CrypteckAmmunition)
	{
		Pickable = false;
		Hittable = false;
		Collider->SetPhysicsAngularVelocityInDegrees(FVector(0, 0, 1) * Rotation);
	}
	else
	{
		Pickable = false;
		Hittable = true;
		Collider->SetWorldScale3D(FVector(.3f, .3f, .3f));
	}

	Collider->SetPhysicsLinearVelocity(NewDirection * Speed);
}

void ASting::OnBeingSucked(const UDustpan & Dustpan)
{
	if (!CanBeSucked())
		return;

	FVector Location = Dustpan.GetComponentLocation();
	FVector Direction;

	//We calculate vector from me to dustpan
	Direction = Location - GetActorLocation();
	Direction.Normalize();

	//If we are close
	if (FVector::Dist(GetActorLocation(), Location) <= Dustpan.GetSuckedDistance())
	{
		Dustpan.GetPlayerManager()->GiveSomeAlternativeAmmo(1);

		IsSucked = true;
	}
	else
	{
		//We apply force to me
		Collider->SetPhysicsLinearVelocity(Direction * Dustpan.GetSuckVelocity());
	}
}

void ASting::OnObjectExitsPool()
{
	Pickable = Hittable = false;
	Collider->SetPhysicsLinearVelocity(FVector::ZeroVector);
	Collider->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	Collider->SetEnableGravity(false);
	Collider->SetRelativeRotation(FRotator(270, 0, 0));
	Collider->SetWorldScale3D(FVector(.3f, 1, 1));
	Collider->BodyInstance.bLockRotation = false;
	Collider->BodyInstance.bLockTranslation = false;
}

void ASting::OnObjectEnterPool()
{
	Pickable = Hittable = false;
	Collider->SetPhysicsLinearVelocity(FVector::ZeroVector);
	Collider->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	Collider->SetEnableGravity(false);
	Collider->SetRelativeRotation(FRotator(270, 0, 0));
	Collider->SetWorldScale3D(FVector(.3f, 1, 1));
	Collider->BodyInstance.bLockRotation = true;
	Collider->BodyInstance.bLockTranslation = true;
	SetActorLocation(OwnerPool->GetActorLocation());
}

