#include "Ability.h"
#include "Camera/CameraComponent.h"
#include "ZaProjectCharacter.h"
#include "AbilityActor.h"
#include "PlayerStats.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "StatusEffectsManager.h"


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
    PlayerStats = Owner->FindComponentByClass<UPlayerStats>();
    CameraComponent = Owner->FindComponentByClass<UCameraComponent>();
    SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();

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

    if (!CameraComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Camera component not found"));
        return;
    }
    
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Skeletal mesh component not found"));
        return;
    }

    FVector SpawnLocation = SkeletalMeshComponent->GetSocketLocation(AbilityFromSocket) + Owner->GetActorForwardVector() * 20.f;
    FVector SpawnDirection = CameraComponent->GetForwardVector();
    FVector SpawnEnd = SpawnDirection * Range + SpawnLocation;

    FHitResult HitResult;
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(Owner);
    TraceParams.bTraceComplex = true;
    TraceParams.bReturnPhysicalMaterial = true;

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, SpawnLocation, SpawnEnd, ECC_Camera, TraceParams);

    FColor DebugLineColor = bHit ? FColor::Red : FColor::Green;
    float DebugLineLifetime = 5.0f;
    uint8 DepthPriority = 0;
    float LineThickness = 2.0f;
    DrawDebugLine(GetWorld(), SpawnLocation, SpawnEnd, DebugLineColor, true, DebugLineLifetime, DepthPriority, LineThickness);

    // Call the function to spawn the beam VFX
    SpawnHitscanBeamVFX(SpawnLocation, SpawnEnd);

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
                UStatusEffectsManager* OtherPlayerStatusEffects = HitResult.GetActor()->FindComponentByClass<UStatusEffectsManager>();

                if (OtherPlayerStats)
                {
                    OtherPlayerStats->TakeDamage(Damage, GetOwner()->GetInstigatorController(), GetOwner());
                }

                if (OtherPlayerStatusEffects)
                {
                    if (Burns)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Ability Burns."));
                        OtherPlayerStatusEffects->HandleBurn(AbilityName, BurnStacks, NumberOfBurnStacks, BurnDamagePerTick, BurnTickDuration, BurnNumberOfTicks);
                    }
                    if (Wets)
                    {
                        OtherPlayerStatusEffects->AddWetStatusEffect(WetDuration);
                    }
                    if (Poisons)
                    {
                        OtherPlayerStatusEffects->HandlePoison(AbilityName, PoisonStacks, NumberOfPoisonStacks, PoisonDamagePerTick, PoisonTickDuration, PoisonNumberOfTicks);
                    }
                    if (AntiHeals)
                    {
                        OtherPlayerStatusEffects->HandleAntihealModifier(AbilityName, AntihealStacks, NumberOfStacks, AntiHealPercentageToAdd, AntiHealsDuration);
                    }
                    if (Heals)
                    {
                        OtherPlayerStatusEffects->HandleHeal(HealAmountPerTick, HealTickDuration, HealNumberOfTicks);
                    }
                    if (SpeedModifier)
                    {
                        OtherPlayerStatusEffects->HandleSpeedModifier(AbilityName, SpeedStacks, NumberOfSpeedModStacks, SpeedModifier, SpeedModifierDuration);
                    }
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


void UAbility::Server_SpawnEmitter_Implementation(const FVector& SpawnLocation, const FRotator& Rotation)
{
    Multicast_SpawnEmitter(SpawnLocation, Rotation);
}

bool UAbility::Server_SpawnEmitter_Validate(const FVector& SpawnLocation, const FRotator& Rotation)
{
    return true;
}

void UAbility::SpawnHitscanBeamVFX(const FVector& Start, const FVector& End)
{
    if (HitscanBeamVFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitscanBeamVFX, Start, FRotator::ZeroRotator, FVector::OneVector, true, EPSCPoolMethod::None, true);
    }
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

    if (!CameraComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Camera component not found"));
        return;
    }

    USkeletalMeshComponent* SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Skeletal mesh component not found"));
        return;
    }

    FVector SpawnLocation = SkeletalMeshComponent->GetSocketLocation(AbilityFromSocket) + Owner->GetActorForwardVector() * 20.f;
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

    Projectile->Damage = Damage;

    UPrimitiveComponent* ProjectileCollision = Projectile->FindComponentByClass<UPrimitiveComponent>();
    if (ProjectileCollision)
    {
        //ProjectileCollision->MoveIgnoreActors.Add(GetOwner());
    }
}

// Replicate any relevant properties here
void UAbility::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}