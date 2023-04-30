// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStats.h"
#include "ZaProjectCharacter.h"
#include "AbilityActor.h"
#include "Kismet/GameplayStatics.h"
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
	EAbilityType AbilityType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	EAbilityCastType CastType;

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

	// Hitscan

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hitscan")
	float Range;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan")
	UParticleSystem* HitEmitter;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SpawnEmitter(FVector SpawnLocation, FRotator Rotation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnEmitter(FVector SpawnLocation, FRotator Rotation);

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
};