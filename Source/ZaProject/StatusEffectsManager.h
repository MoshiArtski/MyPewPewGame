// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "PlayerStats.h"
#include "StatusEffectsManager.generated.h"


USTRUCT(BlueprintType)
struct FStatusEffectData
{
	GENERATED_BODY()

	int MaxStacks;
	int StackCounter;
	int DamagePerTick;
	float TickDuration;
	int NumberOfTicks;

	FStatusEffectData()
		: MaxStacks(0), StackCounter(0), DamagePerTick(0), TickDuration(0.0f), NumberOfTicks(0)
	{
	}

	FStatusEffectData(int _MaxStacks, int _StackCounter, int _DamagePerTick, float _TickDuration, int _NumberOfTicks)
		: MaxStacks(_MaxStacks), StackCounter(_StackCounter), DamagePerTick(_DamagePerTick), TickDuration(_TickDuration), NumberOfTicks(_NumberOfTicks)
	{
	}
};

struct FStatusEffectModifier
{
	int NMaxStacks;
	int NStackCounter;
	float Modifier;
	float ModifierDuration;
	float TotMod;
	FName Type;
	FTimerHandle TimerHandle;

	FStatusEffectModifier() : NMaxStacks(0), NStackCounter(0), Modifier(0.0f), ModifierDuration(0.0f), TotMod(0.0f), Type(), TimerHandle() {}
	FStatusEffectModifier(int InMaxStacks, int InStackCounter, float InModifier, float InModifierDuration, float InTotMod, FName InType, const FTimerHandle& InTimerHandle)
		: NMaxStacks(InMaxStacks), NStackCounter(InStackCounter), Modifier(InModifier), ModifierDuration(InModifierDuration), TotMod(InTotMod), Type(InType), TimerHandle(InTimerHandle) {}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZAPROJECT_API UStatusEffectsManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Gameplay Tag
	UStatusEffectsManager();

	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags")
	void AddGameplayTagToContainer(const FGameplayTag& TagToAdd);

	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags")
	void RemoveGameplayTagFromContainer(const FGameplayTag& TagToRemove);

	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags")
	void AddTimedGameplayTagToContainer(const FGameplayTag& TagToAdd, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags")
	void HandleTimedGameplayTagRemoval(const FGameplayTag& TagToRemove);

	// Status Effects
	UFUNCTION(BlueprintCallable, Category = "StatusEffects")
	void HandleBurn(const FName& AbilityName, bool BurnStacks, int NumberOfStacks, int _BurnDamagePerTick, float _BurnTickDuration, int _BurnNumberOfTicks);

	UFUNCTION(BlueprintCallable, Category = "StatusEffects")
	void HandlePoison(const FName& AbilityName, bool PoisonStacks, int NumberOfStacks, int _PoisonDamagePerTick, float _PoisonTickDuration, int _PoisonNumberOfTicks);

	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void AddWetStatusEffect(int Duration);

	//UFUNCTION(BlueprintCallable, Category = "Status Effects")
	//void AddAntiHealStatusEffect(float AntiHealPercentageToAdd, float Duration);

	//UFUNCTION(BlueprintCallable, Category = "Status Effects")
	//void HandleAntiHealEffect();

	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void HandleHeal(int HealAmountPerTick, float HealTickDuration, int HealNumberOfTicks);

	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void ApplyHealEffect();

	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void HandleSpeedModifier(const FName& AbilityName, bool SpeedStacks, int NumberOfStacks, int _SpeedModifier, float SpeedModifierDuration);

	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void HandleAntihealModifier(const FName& AbilityName, bool AntihealStacks, int NumberOfStacks, int _AntihealModifier, float AntihealDuration);

	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void HandleStatusEffect(const FName& StatusEffectName, const FName& AbilityName, bool Stacks, int NumberOfStacks, float _DamagePerTick, float _TickDuration, int _NumberOfTicks, bool& IsAffected);
	void HandleStatusEffect(const FName& StatusEffectName, const FName& AbilityName, bool Stacks, int NumberOfStacks, float Effect, float Duration, bool& IsAffected);

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Tags")
	FGameplayTagContainer GameplayTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	bool IsBurning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	bool IsWet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	bool IsPoisoned;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	bool IsHealing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	bool IsAntiHealed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects")
	bool IsSpeedModified;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	// Getters
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	FORCEINLINE bool GetIsBurning() const { return IsBurning; }

	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	FORCEINLINE bool GetIsWet() const { return IsWet; }

	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	FORCEINLINE bool GetIsPoisoned() const { return IsPoisoned; }

	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	FORCEINLINE bool GetIsAntiHealed() const { return IsAntiHealed; }

	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	FORCEINLINE bool GetIsSpeedModified() const { return IsSpeedModified; }

	// Setters
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void SetIsBurning(bool bBurning) { IsBurning = bBurning; }

	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void SetIsWet(bool bWet) { IsWet = bWet; }

	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void SetIsPoisoned(bool bPoisoned) { IsPoisoned = bPoisoned; }

	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void SetIsAntiHealed(bool bHealing) { IsAntiHealed = bHealing; }

	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void SetIsSpeedModified(bool bSpeedMod) { IsSpeedModified = bSpeedMod; }

private:
	AActor* Owner;
	UPlayerStats* PlayerStats;
	//Status Effect
	UFUNCTION()
	void ApplyStatusEffect(const FName& StatusEffectName, const FName& AbilityName, bool& IsAffected);
	TMap<FName, FTimerHandle> StatusEffectTimerHandles;
	TMap <FName, FStatusEffectData> StatusEffectDataMap;
	int MaxStacks;
	int StackCounter;
	int DamagePerTick;
	float TickDuration;
	int NumberOfTicks;
	TMap<FName, FStatusEffectModifier> StatusEffectModifierDataMap;

	//Antiheal
	FTimerHandle AntiHealTimerHandle;
	int32 AntiHealStacks;
    float AntiHealPercentage;
	void UpdateAntiHeal();
	

	//Heal
	FTimerHandle HealTickHandle;
	int CurrentHealPerTick;
	float CurrentHealTickDuration;
	int CurrentHealNumberOfTicks;

	//Speed Modifier
	TMap<float, FTimerHandle> SpeedModifierHandles;
	int32 SpeedModifierInstanceID;
	int32 SpeedModifierStacks;
	float SpeedModifierPercentage;
	float OriginalSpeed;
	float SpeedHandle;
	TMap<FName, float> SpeedHandles;
	void UpdatePlayerSpeed();
	UFUNCTION()
	void RemoveStatusEffect(const FName& StatusEffectName, const FName& AbilityName, bool& IsAffected);
	UFUNCTION()
	void ApplyModiferStatusEffect(const FName& StatusEffectName, const FName& AbilityName, bool& IsAffected);
};
