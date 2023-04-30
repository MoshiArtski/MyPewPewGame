#include "Ability.h"
#include "Camera/CameraComponent.h"
#include "ZaProjectCharacter.h"
#include "AbilityActor.h"
#include "PlayerStats.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"


UAbility::UAbility()
{
	PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);

    // Set default values for the projectile properties
    ProjectileClass = AAbilityActor::StaticClass();
}

void UAbility::BeginPlay()
{
	Super::BeginPlay();

    Owner = Cast<AZaProjectCharacter>(GetOwner());

    EType AbilityElementType = EType::None;
    EType OwnerType1 = Owner->FindComponentByClass<UPlayerStats>()->GetType1();
    EType OwnerType2 = Owner->FindComponentByClass<UPlayerStats>()->GetType2();

    if (OwnerType1 == AbilityElementType || OwnerType2 == AbilityElementType && DealsSTAB)
    {
        Damage = BaseDamage * STABDmgBoost;
    }
    else
    {
        Damage = BaseDamage;
    }
}

void UAbility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAbility::UseAbility_Implementation()
{
    FString healthMessage = FString::Printf(TEXT("Using Ability"), *GetFName().ToString());
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
    if (UseStamina())
    {
        switch (AbilityType)
        {
        case EAbilityType::Projectile:
            if (GetOwner()->GetLocalRole() < ROLE_Authority)
            {
                Server_ProjectileFire();
            }
            else
            {
                ProjectileFire();
            }
            break;
        case EAbilityType::Hitscan:
            if (GetOwner()->GetLocalRole() < ROLE_Authority)
            {
                Server_HitscanFire();
            }
            else
            {
                HitscanFire();
            }
            break;
        default:
            break;
        }
    }
}

bool UAbility::UseStamina()
{
	UPlayerStats* PlayerStats = Cast<UPlayerStats>(GetOwner()->GetComponentByClass(UPlayerStats::StaticClass()));

	if (PlayerStats && Cost < PlayerStats->GetCurrentStamina())
	{
		PlayerStats->UseStamina(Cost);
		return true;
	}
	return false;
}

void UAbility::HitscanFire()
{
    if (!Owner) 
    {
        UE_LOG(LogTemp, Error, TEXT("Owner not found"));
        return;
    }

    UCameraComponent* CameraComponent = Owner->FindComponentByClass<UCameraComponent>();
    if (!CameraComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Camera component not found"));
        return;
    }

    FVector SpawnLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 20.f;
    FVector SpawnDirection = CameraComponent->GetForwardVector();
    FVector SpawnEnd = SpawnDirection * Range + SpawnLocation;

    // Adding line trace by channel
    FHitResult HitResult;
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(Owner);
    TraceParams.bTraceComplex = true;
    TraceParams.bReturnPhysicalMaterial = true;

    // Perform line trace
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, SpawnLocation, SpawnEnd, ECC_Camera, TraceParams);

    // Draw debug line
    FColor DebugLineColor = bHit ? FColor::Red : FColor::Green;
    float DebugLineLifetime = 5.0f;
    uint8 DepthPriority = 0;
    float LineThickness = 2.0f;
    DrawDebugLine(GetWorld(), SpawnLocation, SpawnEnd, DebugLineColor, true, DebugLineLifetime, DepthPriority, LineThickness);

    if (bHit)
    {
        UE_LOG(LogTemp, Log, TEXT("Hit Actor: %s"), *HitResult.GetActor()->GetName());

            if (HitEmitter)
            {
                FRotator Rotation = FRotationMatrix::MakeFromX(HitResult.ImpactNormal).Rotator();
                if (GetOwner()->GetLocalRole() < ROLE_Authority)
                {
                    Server_SpawnEmitter(HitResult.ImpactPoint, Rotation);
                }
                else
                {
                    Multicast_SpawnEmitter(HitResult.ImpactPoint, Rotation);
                }

                UPlayerStats* OtherPlayerStats = HitResult.GetActor()->FindComponentByClass<UPlayerStats>();

                if (OtherPlayerStats)
                {
                    OtherPlayerStats->TakeDamage(Damage, GetOwner()->GetInstigatorController(), GetOwner());
                }
            }
    }
}

void UAbility::Server_HitscanFire_Implementation()
{
    if (!ensure(GetOwner()->GetLocalRole() == ROLE_Authority)) return;

    HitscanFire();
}

bool UAbility::Server_HitscanFire_Validate()
{
    return true;
}


void UAbility::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // Replicate any relevant properties here
}

void UAbility::Server_SpawnEmitter_Implementation(FVector SpawnLocation, FRotator Rotation)
{
    Multicast_SpawnEmitter(SpawnLocation, Rotation);
}

bool UAbility::Server_SpawnEmitter_Validate(FVector SpawnLocation, FRotator Rotation)
{
    return true;
}

void UAbility::Multicast_SpawnEmitter_Implementation(FVector SpawnLocation, FRotator Rotation)
{
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEmitter, SpawnLocation, Rotation);
}

void UAbility::Server_ProjectileFire_Implementation()
{
    if (!ensure(GetOwner()->GetLocalRole() == ROLE_Authority)) return;

    ProjectileFire();
}

bool UAbility::Server_ProjectileFire_Validate()
{
    return true;
}

void UAbility::ProjectileFire()
{
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("Owner not found"));
        return;
    }

    UCameraComponent* CameraComponent = Owner->FindComponentByClass<UCameraComponent>();
    if (!CameraComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Camera component not found"));
        return;
    }

    FVector SpawnLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 20.f;
    FVector SpawnDirection = CameraComponent->GetForwardVector();

    // Spawn the projectile
    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    SpawnParameters.Instigator = Owner;
    SpawnParameters.Owner = GetOwner();

    AAbilityActor* Projectile = GetWorld()->SpawnActor<AAbilityActor>(ProjectileClass, SpawnLocation, SpawnDirection.Rotation(), SpawnParameters);

    if (!Projectile)
    {
        UE_LOG(LogTemp, Error, TEXT("Projectile not spawned"));
        return;
    }

    UPrimitiveComponent* ProjectileCollision = Projectile->FindComponentByClass<UPrimitiveComponent>();
    if (ProjectileCollision)
    {
        ProjectileCollision->MoveIgnoreActors.Add(GetOwner());
        UE_LOG(LogTemp, Error, TEXT("Projectile Collision does not exist."));
    }
}