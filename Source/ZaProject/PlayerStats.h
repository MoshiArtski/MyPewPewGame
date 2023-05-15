// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/DamageType.h"

#include "PlayerStats.generated.h"


UENUM(BlueprintType)
enum class EType : uint8
{
	Fire    UMETA(DisplayName = "Fire"),
	Air     UMETA(DisplayName = "Air"),
	Water   UMETA(DisplayName = "Water"),
	Earth   UMETA(DisplayName = "Earth"),
	None    UMETA(DisplayName = "None"),
};

USTRUCT(BlueprintType)
struct FCharacterStats : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (DisplayName = "Index", MakeStructureDefaultValue = "0"))
	int32 Index;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, SaveGame, meta = (DisplayName = "Species"))
	FText Species;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, meta = (DisplayName = "Name"))
	FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, meta = (DisplayName = "Attack", MakeStructureDefaultValue = "0"))
	int32 Attack;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, meta = (DisplayName = "Aura", MakeStructureDefaultValue = "0"))
	int32 Aura;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, meta = (DisplayName = "Defence", MakeStructureDefaultValue = "0"))
	int32 Defence;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, meta = (DisplayName = "AuraDefence", MakeStructureDefaultValue = "0"))
	int32 AuraDefence;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, meta = (DisplayName = "Constitution", MakeStructureDefaultValue = "0"))
	int32 Constitution;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, meta = (DisplayName = "Speed", MakeStructureDefaultValue = "0"))
	int32 Speed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, meta = (DisplayName = "Weight", MakeStructureDefaultValue = "0"))
	int32 Weight;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, SaveGame, meta = (DisplayName = "Type"))
	EType mType1;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, SaveGame, meta = (DisplayName = "Type"))
	EType mType2;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZAPROJECT_API UPlayerStats : public UActorComponent
{
	GENERATED_BODY()

public:

	 UPlayerStats();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;


	/*****************************************Stats*****************************************/

protected:

	//Health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float MaxHealth;


	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	UFUNCTION()
	void OnRep_CurrentHealth();

	void OnHealthUpdate();

	// Stamina
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float MaxStamina;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentStamina)
	float CurrentStamina;

	UFUNCTION()
	void OnRep_CurrentStamina();

	void OnStaminaUpdate();

	FTimerHandle StaminaRegenTimerHandle;

	// Types
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	EType Type1;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	EType Type2;

	UPROPERTY(EditDefaultsOnly, Category = "Stat Info")
	UDataTable* StatInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes", meta = (ShowOnlyInnerProperties))
	FCharacterStats CharacterStats;

	//Other Stats 
	UPROPERTY(Replicated)
	int32 Aura;

	UPROPERTY(Replicated)
	int32 AuraDefense;

	UPROPERTY(Replicated)
	int32 Defense;

	UPROPERTY(Replicated)
	int32 Weight;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 Speed;

	// Getters and Setters

public:

	/** Getter for Max Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealth(float healthValue);


	// Events for taking damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	bool bIsDamageable;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageTaken, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TakeDamage(float DamageTaken, AController* EventInstigator, AActor* DamageCauser);


	/** Getter for Max Stamina.*/
	UFUNCTION(BlueprintPure, Category = "Stamina")
	FORCEINLINE float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	FORCEINLINE float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetCurrentStamina(float staminaValue);

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void UseStamina(float staminaAmount);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Stamina")
	void Server_UseStamina(float staminaAmount);

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	virtual void RegenerateStamina();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaRegenRate;

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetStaminaRegenRate(float regenRate);

	/** Getter for Types. */
	UFUNCTION(BlueprintPure, Category = "ElementType")
	FORCEINLINE EType GetType1() const { return Type1; }

	UFUNCTION(BlueprintCallable, Category = "ElementType")
	void SetType1(EType NewType1) { Type1 = NewType1; }

	UFUNCTION(BlueprintPure, Category = "ElementType")
	FORCEINLINE EType GetType2() const { return Type2; }

	UFUNCTION(BlueprintCallable, Category = "ElementType")
	void SetType2(EType NewType2) { Type2 = NewType2; }

	/** Getter for Damageable. */
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE bool GetIsDamageable() const { return bIsDamageable; }

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetIsDamageable(bool NewIsDamageble) {  bIsDamageable = NewIsDamageble; }

	/** Getter for Speed.*/
	UFUNCTION(BlueprintPure, Category = "Stats")
	FORCEINLINE float GetSpeed() const { return Speed; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetSpeed(float NewSpeed);

private:

	bool IsLocallyControlledPawn();
};
