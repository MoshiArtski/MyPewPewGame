#include "AbilityActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerStats.h"


AAbilityActor::AAbilityActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Setup components
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->InitSphereRadius(15.0f);
	SphereComponent->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = SphereComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = SphereComponent;
	ProjectileMovementComponent->InitialSpeed = 1500.0f;
	ProjectileMovementComponent->MaxSpeed = 1500.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

	// Register hit event
	SphereComponent->OnComponentHit.AddDynamic(this, &AAbilityActor::OnProjectileHit);

	bReplicates = true;

	MaxImpacts = 1;
	ImpactCounter = 0;
}

void AAbilityActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAbilityActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAbilityActor::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{

	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	// Spawn the emitter at the impact location
	if (HitEmitter)
	{
		FVector ImpactLocation = Hit.ImpactPoint;
		FRotator Rotation = FRotationMatrix::MakeFromX(Hit.ImpactNormal).Rotator();
		if (GetOwner()->GetLocalRole() < ROLE_Authority)
		{
			Server_SpawnEmitter(ImpactLocation, Rotation);
		}
		else
		{
			Multicast_SpawnEmitter(ImpactLocation, Rotation);
		}
	}

	UPlayerStats* OtherPlayerStats = OtherActor->FindComponentByClass<UPlayerStats>();
	if (OtherPlayerStats)
	{
		OtherPlayerStats->TakeDamage(Damage, GetOwner()->GetInstigatorController(), GetOwner());
	}
	
	
	// Increment the impact counter
	ImpactCounter++;

	// Destroy the projectile if the maximum number of impacts has been reached
	if (ImpactCounter >= MaxImpacts)
	{
		Destroy();
	}
}

void AAbilityActor::Server_SpawnEmitter_Implementation(FVector SpawnLocation, FRotator Rotation)
{
	Multicast_SpawnEmitter(SpawnLocation, Rotation);
}

bool AAbilityActor::Server_SpawnEmitter_Validate(FVector SpawnLocation, FRotator Rotation)
{
	return true;
}

void AAbilityActor::Multicast_SpawnEmitter_Implementation(FVector SpawnLocation, FRotator Rotation)
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEmitter, SpawnLocation, Rotation);
}
