#include "StatusEffectsManager.h"
#include "ZaProjectCharacter.h"
#include "PlayerStats.h"

UStatusEffectsManager::UStatusEffectsManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStatusEffectsManager::BeginPlay()
{
	Super::BeginPlay();

    Owner = GetOwner();
	PlayerStats = Owner->FindComponentByClass<UPlayerStats>();
	OriginalSpeed = PlayerStats->GetSpeed();
}

void UStatusEffectsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// Add and remove gameplay tags

void UStatusEffectsManager::AddGameplayTagToContainer(const FGameplayTag& TagToAdd)
{
	if (TagToAdd.IsValid())
	{
		GameplayTags.AddTag(TagToAdd);
		UE_LOG(LogTemp, Warning, TEXT("Tag added: %s"), *TagToAdd.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid tag: %s"), *TagToAdd.ToString());
	}
}


void UStatusEffectsManager::RemoveGameplayTagFromContainer(const FGameplayTag& TagToRemove)
{
	if (TagToRemove.IsValid())
	{
		GameplayTags.RemoveTag(TagToRemove);
	}
}

void UStatusEffectsManager::AddTimedGameplayTagToContainer(const FGameplayTag& TagToAdd, float Duration)
{
	if (TagToAdd.IsValid() && Duration > 0.0f)
	{
		GameplayTags.AddTag(TagToAdd);
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, TagToAdd]() {
			HandleTimedGameplayTagRemoval(TagToAdd);
			}, Duration, false);
	}
}

void UStatusEffectsManager::HandleTimedGameplayTagRemoval(const FGameplayTag& TagToRemove)
{
	if (TagToRemove.IsValid())
	{
		GameplayTags.RemoveTag(TagToRemove);
	}
}

//Handle Ticking Status Effects

void UStatusEffectsManager::HandleStatusEffect(const FName& StatusEffectName, const FName& AbilityName, bool Stacks, int NumberOfStacks, float _DamagePerTick, float _TickDuration, int _NumberOfTicks, bool& IsAffected)
{
	FGameplayTag ImmunityTag = FGameplayTag::RequestGameplayTag(FName(*FString::Printf(TEXT("StatusEffect.ImmuneTo%s"), *StatusEffectName.ToString())));
	if (GameplayTags.HasTag(ImmunityTag))
	{
		return;
	}

	if (_NumberOfTicks <= 0)
	{
		return;
	}

	if (!StatusEffectTimerHandles.Contains(AbilityName))
	{
		FTimerHandle StatusEffectTimer;
		FGameplayTag StatusEffectTag = FGameplayTag::RequestGameplayTag(FName(*FString::Printf(TEXT("StatusEffect.%s"), *StatusEffectName.ToString())));
		AddGameplayTagToContainer(StatusEffectTag);
		FStatusEffectData NewStatusEffectData(NumberOfStacks, 0, _DamagePerTick, _TickDuration, _NumberOfTicks);
		StatusEffectTimerHandles.Add(AbilityName, MoveTemp(StatusEffectTimer));
		StatusEffectDataMap.Add(AbilityName, NewStatusEffectData);
		IsAffected = true;
	}

	FStatusEffectData& EffectData = StatusEffectDataMap[AbilityName];

	if (Stacks && EffectData.StackCounter >= EffectData.MaxStacks)
	{
		return;
	}

	if (Stacks)
	{
		EffectData.StackCounter++;
	}
	else
	{
		EffectData.StackCounter = 1;
	}

	EffectData.NumberOfTicks = _NumberOfTicks;
	FTimerDelegate ApplyStatusEffectDelegate;
	ApplyStatusEffectDelegate.BindUFunction(this, FName("ApplyStatusEffect"), StatusEffectName, AbilityName, IsAffected);
	ApplyStatusEffect(StatusEffectName, AbilityName, IsAffected);
	GetWorld()->GetTimerManager().SetTimer(StatusEffectTimerHandles[AbilityName], ApplyStatusEffectDelegate, _TickDuration, false);
}

void UStatusEffectsManager::ApplyStatusEffect(const FName& StatusEffectName, const FName& AbilityName, bool& IsAffected)
{
	UE_LOG(LogTemp, Warning, TEXT("Apply Status Called."));

	if (!StatusEffectTimerHandles.Contains(AbilityName) && !StatusEffectDataMap.Contains(AbilityName) && !PlayerStats)
	{
		return;
	}
	
	FStatusEffectData& EffectData = StatusEffectDataMap[AbilityName];

	PlayerStats->TakeDamage(EffectData.DamagePerTick * EffectData.StackCounter, nullptr, Owner);

	if (EffectData.NumberOfTicks > 1)
	{
	   FTimerDelegate ApplyStatusEffectDelegate;
	   ApplyStatusEffectDelegate.BindUFunction(this, FName("ApplyStatusEffect"), StatusEffectName, AbilityName, IsAffected);
	   GetWorld()->GetTimerManager().SetTimer(StatusEffectTimerHandles[AbilityName], ApplyStatusEffectDelegate, EffectData.TickDuration, false);
	   EffectData.NumberOfTicks--;
	}
	else
	{
	   FGameplayTag StatusEffectTag = FGameplayTag::RequestGameplayTag(FName(*FString::Printf(TEXT("StatusEffect.%s"), *StatusEffectName.ToString())));
	   RemoveGameplayTagFromContainer(StatusEffectTag);
	   StatusEffectTimerHandles.Remove(AbilityName);
	   StatusEffectDataMap.Remove(AbilityName);
	   IsAffected = false;
	}
}

//Handle Modifier Status Effects

void UStatusEffectsManager::HandleStatusEffect(const FName& StatusEffectName, const FName& AbilityName, bool Stacks, int NumberOfStacks, float EffectPower, float Duration, bool& IsAffected)
{
	UE_LOG(LogTemp, Warning, TEXT("EffectPower: %f"), EffectPower);

	FGameplayTag ImmunityTag = FGameplayTag::RequestGameplayTag(FName(*FString::Printf(TEXT("StatusEffect.ImmuneTo%s"), *StatusEffectName.ToString())));
	if (GameplayTags.HasTag(ImmunityTag))
	{
		return;
	}

	if (!StatusEffectModifierDataMap.Contains(AbilityName))
	{
		FGameplayTag StatusEffectTag = FGameplayTag::RequestGameplayTag(FName(*FString::Printf(TEXT("StatusEffect.%s"), *StatusEffectName.ToString())));
		AddGameplayTagToContainer(StatusEffectTag);
		FTimerHandle StatusEffectTimer;
		FStatusEffectModifier NewStatusEffectData(NumberOfStacks, 0, EffectPower, Duration, 0.0f, StatusEffectName, MoveTemp(StatusEffectTimer));
		StatusEffectModifierDataMap.Add(AbilityName, NewStatusEffectData);
		IsAffected = true;
	}

	if (!StatusEffectModifierDataMap.Contains(AbilityName))
	{
		return;
	}

		FStatusEffectModifier& EffectData = StatusEffectModifierDataMap[AbilityName];

	if (Stacks && EffectData.NStackCounter >= EffectData.NMaxStacks)
	{
		return;
	}

	if (Stacks)
	{
		EffectData.NStackCounter++;
	}
	else
	{
		EffectData.NStackCounter = 1;
	}

	EffectData.ModifierDuration = Duration;
	StatusEffectModifierDataMap[AbilityName].TotMod = StatusEffectModifierDataMap[AbilityName].Modifier * StatusEffectModifierDataMap[AbilityName].NStackCounter;
	ApplyModiferStatusEffect(StatusEffectName, AbilityName, IsAffected);
}

void UStatusEffectsManager::ApplyModiferStatusEffect(const FName& StatusEffectName, const FName& AbilityName, bool& IsAffected)
{
	UE_LOG(LogTemp, Warning, TEXT("Apply Status Called."));
	FStatusEffectModifier EffectData = StatusEffectModifierDataMap[AbilityName];
	if (PlayerStats)
	{
		if (StatusEffectName == "SpeedModifier")
		{
			UpdatePlayerSpeed();
		}
		if (StatusEffectName == "AntiHealed")
		{
			UpdateAntiHeal();
		}

		// Schedule the removal of the status effect after the given duration
		FTimerDelegate RemoveStatusEffectDelegate;
		RemoveStatusEffectDelegate.BindUFunction(this, FName("RemoveStatusEffect"), StatusEffectName, AbilityName, IsAffected);
		GetWorld()->GetTimerManager().SetTimer(EffectData.TimerHandle, RemoveStatusEffectDelegate, EffectData.ModifierDuration, false);
	}
}

void UStatusEffectsManager::RemoveStatusEffect(const FName& StatusEffectName, const FName& AbilityName, bool& IsAffected)
{
	if (StatusEffectModifierDataMap.Contains(AbilityName) && PlayerStats)
	{
		FStatusEffectModifier& EffectData = StatusEffectModifierDataMap[AbilityName];
		{
			FGameplayTag StatusEffectTag = FGameplayTag::RequestGameplayTag(FName(*FString::Printf(TEXT("StatusEffect.%s"), *StatusEffectName.ToString())));
			RemoveGameplayTagFromContainer(StatusEffectTag);
			StatusEffectTimerHandles.Remove(AbilityName);
			StatusEffectModifierDataMap.Remove(AbilityName);
			IsAffected = false;
			if (StatusEffectName == "SpeedModifier")
			{
				UpdatePlayerSpeed(); 
			}
			if (StatusEffectName == "AntiHealed")
			{
				UE_LOG(LogTemp, Warning, TEXT("Yush"));
				UpdateAntiHeal();
			}
		}
	}
}

void UStatusEffectsManager::UpdatePlayerSpeed()
{
	float TotalSpeedModifier = 0.0f;

	for (const auto& EffectDataPair : StatusEffectModifierDataMap)
	{
		const FStatusEffectModifier& EffectData = EffectDataPair.Value;

		if (EffectData.Type == FName(TEXT("SpeedModifier")))
		{
			TotalSpeedModifier += EffectData.TotMod;
		}
	}
	PlayerStats->SetSpeed(OriginalSpeed * (1.0f + TotalSpeedModifier));
}

void UStatusEffectsManager::UpdateAntiHeal()
{
	float TotalAntiHeal = 0.0f;

	for (const auto& EffectDataPair : StatusEffectModifierDataMap)
	{
		const FStatusEffectModifier& EffectData = EffectDataPair.Value;

		if (EffectData.Type == FName(TEXT("Antiheal")))
		{ 
		TotalAntiHeal += EffectData.TotMod;
	    }
	}

	AntiHealPercentage /= (1.0f - AntiHealPercentage);
}

//Wet Add for Duration

void UStatusEffectsManager::AddWetStatusEffect(int Duration)
{
	FGameplayTag WetTag = FGameplayTag::RequestGameplayTag(FName("StatusEffect.Wet"));
	AddGameplayTagToContainer(WetTag);
	IsWet = true; 
	FGameplayTag ImmuneToBurned = FGameplayTag::RequestGameplayTag(FName("StatusEffect.ImmuneToBurned"));
	AddGameplayTagToContainer(ImmuneToBurned);

	FTimerHandle WetTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(WetTimerHandle, [this, WetTag, ImmuneToBurned]() {
		RemoveGameplayTagFromContainer(WetTag); IsWet = false; RemoveGameplayTagFromContainer(ImmuneToBurned);
		}, Duration, false);
}

//Healing

void UStatusEffectsManager::HandleHeal(int HealAmountPerTick, float HealTickDuration, int HealNumberOfTicks)
{
	if (HealNumberOfTicks <= 0)
	{
		return;
	}

	if (IsHealing)
	{
		if (HealAmountPerTick > CurrentHealPerTick)
		{
			CurrentHealPerTick = HealAmountPerTick;
		}

		if (HealTickDuration < CurrentHealTickDuration)
		{
			CurrentHealTickDuration = HealTickDuration;
		}

		if (HealNumberOfTicks > CurrentHealNumberOfTicks)
		{
			CurrentHealNumberOfTicks = HealNumberOfTicks;
		}

		GetWorld()->GetTimerManager().ClearTimer(HealTickHandle);
	}
	else
	{
		IsHealing = true;
		FGameplayTag HealingTag = FGameplayTag::RequestGameplayTag(FName("StatusEffect.Healing"));
		AddGameplayTagToContainer(HealingTag);
		CurrentHealPerTick = HealAmountPerTick;
		CurrentHealTickDuration = HealTickDuration;
		CurrentHealNumberOfTicks = HealNumberOfTicks;
	}

	ApplyHealEffect();
}

void UStatusEffectsManager::ApplyHealEffect()
{
	if (PlayerStats)
	{
		float HealAmount = CurrentHealPerTick * (1.0f - AntiHealPercentage);
		PlayerStats->TakeDamage(-HealAmount);
	}

	if (CurrentHealNumberOfTicks > 1)
	{
		GetWorld()->GetTimerManager().SetTimer(HealTickHandle, [this]() {
			CurrentHealNumberOfTicks--;
			ApplyHealEffect();
			}, CurrentHealTickDuration, false);
	}
	else
	{
		IsHealing = false;
		FGameplayTag HealingTag = FGameplayTag::RequestGameplayTag(FName("StatusEffect.Healing"));
		RemoveGameplayTagFromContainer(HealingTag);
	}
}

void UStatusEffectsManager::HandleBurn(const FName& AbilityName, bool BurnStacks, int NumberOfStacks, int _BurnDamagePerTick, float _BurnTickDuration, int _BurnNumberOfTicks)
{
   HandleStatusEffect(FName("Burned"), AbilityName, BurnStacks, NumberOfStacks, _BurnDamagePerTick, _BurnTickDuration, _BurnNumberOfTicks, IsBurning);
}

void UStatusEffectsManager::HandlePoison(const FName& AbilityName, bool PoisonStacks, int NumberOfStacks, int _PoisonDamagePerTick, float _PoisonTickDuration, int _PoisonNumberOfTicks)
{
   HandleStatusEffect(FName("Poisoned"), AbilityName, PoisonStacks, NumberOfStacks, _PoisonDamagePerTick, _PoisonTickDuration, _PoisonNumberOfTicks, IsPoisoned);
}

void UStatusEffectsManager::HandleSpeedModifier(const FName& AbilityName, bool SpeedStacks, int NumberOfStacks, int _SpeedModifier, float SpeedModifierDuration)
{
   HandleStatusEffect(FName("SpeedModifier"), AbilityName, SpeedStacks, NumberOfStacks, _SpeedModifier, SpeedModifierDuration, IsSpeedModified);
}

void UStatusEffectsManager::HandleAntihealModifier(const FName& AbilityName, bool AntihealStacks, int NumberOfStacks, int _AntihealModifier, float AntihealDuration)
{
	HandleStatusEffect(FName("Antiheal"), AbilityName, AntihealStacks, NumberOfStacks, _AntihealModifier, AntihealDuration, IsAntiHealed);
}