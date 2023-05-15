// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStats.h"
#include "ZaProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "AbilityActor.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerStats.h"
#include "Ability.generated.h"


UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	Projectile,
	Hitscan,
	Placed,
	Self,
};

UENUM(BlueprintType)
enum class EAbilityCastType : uint8
{
	Charged,
	Instant,
};

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ZAPROJECT_API UAbility : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAbility();

protected:
	// Base Info
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	FName AbilityName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	EAbilityType AbilityType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	EAbilityCastType CastType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	EType AbilityElementType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	int BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	bool DealsSTAB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float STABDmgBoost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float Cost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FName AbilityFromSocket;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float FireRate;



	// Other Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	bool Burns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects", meta = (ToolTip = "Does this Burn Stack with itself."))
	bool BurnStacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects", meta = (ToolTip = "Number of Burn Stacks"))
	int NumberOfBurnStacks;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	int BurnDamagePerTick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	float BurnTickDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	int BurnNumberOfTicks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	bool Wets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	float WetDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	bool Poisons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects", meta = (ToolTip = "Does this Poison Stack with itself."))
	bool PoisonStacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects", meta = (ToolTip = "Number of Poison Stacks"))
	int NumberOfPoisonStacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	int PoisonDamagePerTick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	float PoisonTickDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	int PoisonNumberOfTicks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	bool AntiHeals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	float AntiHealPercentageToAdd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects", meta = (ToolTip = "Does this Antiheal Stack with itself."))
	bool AntihealStacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects", meta = (ToolTip = "Number of Antiheal Stacks"))
	int NumberOfStacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	float  AntiHealsDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	bool Heals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	int HealAmountPerTick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	float HealTickDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	int HealNumberOfTicks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects", meta = (ToolTip = "Does this Speed Modifier Stack with itself."))
	bool SpeedStacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects", meta = (ToolTip = "Number of Stacks"))
	int NumberOfSpeedModStacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	bool SpeedModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	float SpeedModifierPercentageToAdd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	float  SpeedModifierDuration;

	// Hitscan

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hitscan")
	float Range;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan")
	UParticleSystem* HitEmitter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UParticleSystem* HitscanBeamVFX;

	void SpawnHitscanBeamVFX(const FVector& Start, const FVector& End);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SpawnEmitter(const FVector& SpawnLocation, const FRotator& Rotation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnEmitter(const FVector& SpawnLocation, const FRotator& Rotation);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_HitscanFire();

	UFUNCTION(BlueprintCallable, Category = "Ability")
	void HitscanFire();

	// Projectile

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ProjectileFire();


	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ProjectileFire();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AAbilityActor> ProjectileClass;


public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	void UseAbility();
	virtual void UseAbility_Implementation();

private:
	bool UseStamina();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override; 
	int Damage;
	AZaProjectCharacter* Owner;
	UPlayerStats* PlayerStats;
	UCameraComponent* CameraComponent;
	USkeletalMeshComponent* SkeletalMeshComponent;
};