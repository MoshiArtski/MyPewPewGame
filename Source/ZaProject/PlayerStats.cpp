// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStats.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UPlayerStats::UPlayerStats()
{
}


// Called when the game starts
void UPlayerStats::BeginPlay()
{
	Super::BeginPlay();

	if (StatInfo)
	{
		FCharacterStats* Row = StatInfo->FindRow<FCharacterStats>(FName(*FString::FromInt(0)), "Index");

		if (Row)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Row found in data table"));

			// Initialize 
			MaxHealth = Row->Constitution * 10;
			CurrentHealth = MaxHealth;

			MaxStamina = Row->Constitution * Row->Aura;
			CurrentStamina = MaxStamina;
			StaminaRegenRate = Row->Aura * 10;

			Type1 = Row->mType1;
			Type2 = Row->mType2;

			CharacterStats = *Row;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Row not found in data table"));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("StatInfo is not assigned"));
	}

	// Initialize Stamina Regen Timer
	GetWorld()->GetTimerManager().SetTimer(StaminaRegenTimerHandle, this, &UPlayerStats::RegenerateStamina, 1.0f, true);
	bIsDamageable = true;
}



/******************************** Health Functions ********************************/


void UPlayerStats::OnHealthUpdate()
{
	if (IsLocallyControlledPawn())
	{
		FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (CurrentHealth <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}

	}

	if (GetOwnerRole() == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}

	//Functions that occur on all machines. 
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
}

void UPlayerStats::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void UPlayerStats::SetCurrentHealth(float healthValue)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

float UPlayerStats::TakeDamage(float DamageTaken, AController* EventInstigator, AActor* DamageCauser)
{
	if (!bIsDamageable)
	{
		return 0.f;
	}

	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_TakeDamage(DamageTaken, EventInstigator, DamageCauser);
		return DamageTaken;
	}
	else
	{
		float damageApplied = CurrentHealth - DamageTaken;
		SetCurrentHealth(damageApplied);
		return damageApplied;
	}
}

void UPlayerStats::Server_TakeDamage_Implementation(float DamageTaken, AController* EventInstigator, AActor* DamageCauser)
{
	TakeDamage(DamageTaken, EventInstigator, DamageCauser);
}

bool UPlayerStats::Server_TakeDamage_Validate(float DamageTaken, AController* EventInstigator, AActor* DamageCauser)
{
	return true;
}

/******************************** Stamina Functions ********************************/

void UPlayerStats::OnStaminaUpdate()
{
	// Handle stamina changes
	//if (IsLocallyControlledPawn())
	//{
	//	FString staminaMessage = FString::Printf(TEXT("You now have %f stamina remaining."), CurrentStamina);
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, staminaMessage);
	//}

	if (GetOwnerRole() == ROLE_Authority)
	{
		FString staminaMessage = FString::Printf(TEXT("%s now has %f stamina remaining."), *GetFName().ToString(), CurrentStamina);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, staminaMessage);
	}

	// Any special functionality that should occur as a result of stamina changes should be placed here.
}

void UPlayerStats::OnRep_CurrentStamina()
{
	OnStaminaUpdate();
}

void UPlayerStats::SetCurrentStamina(float staminaValue)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		CurrentStamina = FMath::Clamp(staminaValue, 0.f, MaxStamina);
		OnStaminaUpdate();
	}
}

void UPlayerStats::UseStamina(float staminaAmount)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_UseStamina(staminaAmount);
	}
	else
	{
		float newStamina = CurrentStamina - staminaAmount;
		SetCurrentStamina(newStamina);

		if (GetWorld()->GetTimerManager().IsTimerPaused(StaminaRegenTimerHandle))
		{
			GetWorld()->GetTimerManager().UnPauseTimer(StaminaRegenTimerHandle);
		}
	}
}



void UPlayerStats::Server_UseStamina_Implementation(float staminaAmount)
{
	UseStamina(staminaAmount);
}

bool UPlayerStats::Server_UseStamina_Validate(float staminaAmount)
{
	return true;
}



void UPlayerStats::RegenerateStamina()
{
	if (CurrentStamina < MaxStamina)
	{
		float newStamina = CurrentStamina + (StaminaRegenRate * GetWorld()->GetDeltaSeconds());
		SetCurrentStamina(newStamina);
	}
	else
	{
		// Pause stamina regeneration timer if stamina is full
		GetWorld()->GetTimerManager().PauseTimer(StaminaRegenTimerHandle);
	}
}

void UPlayerStats::SetStaminaRegenRate(float regenRate)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		StaminaRegenRate = regenRate;
	}
}

bool UPlayerStats::IsLocallyControlledPawn()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	return OwnerPawn && OwnerPawn->IsLocallyControlled();
}

/******************************** Replication Functions ********************************/
void UPlayerStats::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerStats, CurrentHealth);
	DOREPLIFETIME(UPlayerStats, CurrentStamina);
}




