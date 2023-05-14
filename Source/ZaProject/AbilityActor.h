#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystem.h"
#include "AbilityActor.generated.h"


UCLASS()
class ZAPROJECT_API AAbilityActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAbilityActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	int32 MaxImpacts;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SpawnEmitter(FVector SpawnLocation, FRotator Rotation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnEmitter(FVector SpawnLocation, FRotator Rotation);


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Function to handle projectile collision
	UFUNCTION()
		void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Projectile Properties")
		int32 Damage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* HitEmitter;

private:
	int32 ImpactCounter;
};
