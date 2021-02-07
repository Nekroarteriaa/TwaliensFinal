// All rights reserved.

#include "SpawnBall.h"
#include "ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "EnemySpawner.h"
#include "BasicWarriorController.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LifeComponent.h"

// Sets default values
ASpawnBall::ASpawnBall() : EnemiesAwaken(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Tags.Add(FName("Crypteck's ball: Spawn"));

	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("SpawnBallCollider"));
	Collider->SetSphereRadius(160);
	Collider->SetNotifyRigidBodyCollision(true);
	Collider->SetCollisionProfileName(FName("UnderGroundPhysicsActor"));
	RootComponent = Collider;

	ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/EditorMeshes/ArcadeEditorSphere.ArcadeEditorSphere"));
	ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Game/Materials/Mat_Water.Mat_Water"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnBallMesh"));
	Mesh->SetCollisionProfileName(FName("NoCollision"));
	Mesh->SetupAttachment(RootComponent);
	if (MeshFinder.Succeeded())
	{
		Mesh->SetStaticMesh(MeshFinder.Object);
	}
	if (MaterialFinder.Succeeded())
	{
		Mesh->SetMaterial(0, MaterialFinder.Object);
	}

	Life = CreateDefaultSubobject<ULifeComponent>(TEXT("SpawnBallLife"));
	DeathDelegate.BindUFunction(this, FName("OnDeath"));
	Life->AddDelegateToDeath(DeathDelegate);
	AddOwnedComponent(Life);
}

// Called when the game starts or when spawned
void ASpawnBall::BeginPlay()
{
	Super::BeginPlay();
}


void ASpawnBall::OnDeath()
{
	Destroy();
}

// Called every frame
void ASpawnBall::Tick(float DeltaTime)
{
	AActor::Tick(DeltaTime);

	if (!EnemiesAwaken)
	{
		TActorIterator<ABasicWarriorController> BWControllerIterator(GetWorld());
		ABasicWarriorController * Controller = (*BWControllerIterator);
		Controller->AlertPlayersInSight(true);
		EnemiesAwaken = true;
	}

	switch (State)
	{
		case ONACTION:
			Action(DeltaTime);
			State = IDLE;
		break;
	}
}

void ASpawnBall::Action(float DeltaSeconds)
{
	for(AEnemySpawner * Spawner : SpawnersLinkedToSpawnBall)
	{
		Spawner->SpawnEnemy();
	}
}

void ASpawnBall::Activate()
{
	if (State == DEACTIVATED)
	{
		State = IDLE;
	}
}

void ASpawnBall::CallReinforcements()
{
	State = ONACTION;
}

