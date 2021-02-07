// All rights reserved.

#include "Weapon.h"
#include "ProjectileA.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "ConstructorHelpers.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Crypteck/Sting.h"
#include "PoolManager.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UWeapon::UWeapon() : Ammunition(20), AltAmmunition(0), BaseFireRate(0.3f), SkillFireRate(0.15f), ProjectileTag("Enemy"), SkillDuration(30), Super()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	FireAudioComponent = CreateDefaultSubobject<UFMODAudioComponent>(TEXT("Gun Audio"));
	FireAudioComponent->SetupAttachment(this);

	WeaponNumber = 0;
}


// Called when the game starts
void UWeapon::BeginPlay()
{
	Super::BeginPlay();

	// ...
	FireRate = BaseFireRate;
	FindPool();
}


// Called every frame
void UWeapon::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWeapon::Fire()
{
	// Attempt to fire a projectile.
	if (BulletPool && Ammunition > 0)
	{
		UWorld * World = GetWorld();

		if (World && !World->GetTimerManager().IsTimerActive(FireRateHandler))
		{	

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = Cast<APawn>(GetOwner());

			// Spawn the projectile at the muzzle.
			AProjectileA* Projectile = Cast<AProjectileA>(BulletPool->GetActorFromPool());
			
			//World->SpawnActor<AProjectileA>(ProjectileClass, GetComponentLocation(), GetComponentRotation(), SpawnParams);
			
			if (Projectile)
			{
				// Set the projectile's initial trajectory.
				Projectile->ResetAllowedTagsToDamage();
				Projectile->AddAllowedTagsToDamage(ProjectileTag);
				//Projectile->SetOwnerPool(BulletPool);
				Projectile->SetActorLocation(GetComponentLocation());
				Projectile->SetActorRotation(GetComponentRotation());
				FVector LaunchDirection = GetForwardVector();
				Projectile->FireInDirection(LaunchDirection);

				if(FireAudioComponent->Event.Get())
				{
					FireAudioComponent->Play();
				}

				//Projectile->Enable();
				--Ammunition;

				World->GetTimerManager().SetTimer(FireRateHandler, FireRate, false);
			}
		}
	}
}


void UWeapon::AltFire()
{
	// Attempt to fire a projectile.
	if (AlternativeBulletPool && AltAmmunition > 0)
	{
		UWorld * World = GetWorld();

		if (World && !World->GetTimerManager().IsTimerActive(FireRateHandler))
		{

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = Cast<APawn>(GetOwner());

			// Spawn the projectile at the muzzle.
			ASting * Projectile = Cast<ASting>(AlternativeBulletPool->GetActorFromPool());

			if (Projectile)
			{
				Projectile->Init(GetForwardVector(), false);
				Projectile->SetActorLocation(GetComponentLocation());
				
				//instead of give weapon rotation we get direction rotator and add it to bullet rotator
				const FRotator NewRotator = UKismetMathLibrary::FindLookAtRotation(Projectile->GetActorLocation(), Projectile->GetActorLocation() + GetForwardVector());

				Projectile->AddActorWorldRotation(NewRotator);//SetActorRotation(NewRotator);

				if (FireAudioComponent->Event.Get())
				{
					FireAudioComponent->Play();
				}

				--AltAmmunition;

				World->GetTimerManager().SetTimer(FireRateHandler, FireRate, false);
			}
		}
	}
}

void UWeapon::WeaponSkill() 
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(SkillHandler)) 
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, TEXT("Skill Activated"));
		FireRate = SkillFireRate;	
		GetWorld()->GetTimerManager().SetTimer(SkillHandler, this, &UWeapon::DeactivateSkill, 1.f, false, SkillDuration);
	}
}

void UWeapon::DeactivateSkill() 
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, TEXT("Skill Desactivated"));
	FireRate = BaseFireRate;
}

void UWeapon::FindPool()
{
	class UWorld* const world = GetWorld();

	if (world)
	{
		for (TActorIterator<APoolManager> Itr(world); Itr; ++Itr)
		{
			APoolManager* FoundPool = *Itr;

			if (FoundPool && FoundPool->Tags.Contains("Bullet"))
			{
				BulletPool = FoundPool;
			}
			else if (FoundPool && FoundPool->Tags.Contains("Alternative Bullet"))
			{
				AlternativeBulletPool = FoundPool;
			}
		}
	}
}

int UWeapon::GetAmmunition() const
{
	return Ammunition; 
}

void UWeapon::ChangeWeapon(int Value)
{
	WeaponNumber += Value;

	if(WeaponNumber >= 2)
	{
		WeaponNumber = 0;
		return;
	}

	if(WeaponNumber < 0)
	{
		WeaponNumber = 1;
		return;
	}
}

void UWeapon::ActivateWeapon()
{
	if(WeaponNumber == 0)
	{
		Fire();
		return;
	}

	if(WeaponNumber == 1)
	{
		AltFire();
		return;
	}
}

int UWeapon::GetAmmo()
{
	if(WeaponNumber == 0)
	{
		return Ammunition;
	}

	if(WeaponNumber == 1)
	{
		return AltAmmunition;
	}

	return NULL;
}

void UWeapon::TransferState(const UWeapon & Other)
{
	//Safety checks
	check(BulletPool == Other.BulletPool);
	check(ProjectileClass == Other.ProjectileClass);
	check(BaseFireRate == Other.BaseFireRate);
	check(SkillFireRate == Other.SkillFireRate);
	check(ProjectileTag == Other.ProjectileTag);

	//copy variables
	Ammunition = Other.Ammunition;
	AltAmmunition = Other.AltAmmunition;
	FireRate = Other.FireRate;

	//copy timers
	//FireRate
	if (GetWorld()->GetTimerManager().IsTimerActive(Other.FireRateHandler))
	{
		float FireRateRemaining = GetWorld()->GetTimerManager().GetTimerRemaining(Other.FireRateHandler);
		GetWorld()->GetTimerManager().SetTimer(FireRateHandler, FireRateRemaining, false);
	}
	//Skill
	if (GetWorld()->GetTimerManager().IsTimerActive(Other.SkillHandler))
	{
		float SkillRemaining = GetWorld()->GetTimerManager().GetTimerRemaining(Other.SkillHandler);
		GetWorld()->GetTimerManager().SetTimer(SkillHandler, this, &UWeapon::DeactivateSkill, SkillRemaining);
	}
}
