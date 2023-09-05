// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;
class UAnimMontage;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual bool CanAttack();
	virtual void Attack(const FInputActionValue& Value);
	virtual void Attack();

	UFUNCTION(BlueprintNativeEvent)
	void Die();

	void DisableCapsule();
	void DisableMeshCollision();
	virtual void HandleDamage(float DamageAmount);
	void DirectionalHitReact(const FVector& ImpactPoint);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	bool IsAlive();

	/** Montage */
	void PlayHitReactMontage(const FName& SectionName);
	void PlayAttackMontage(UAnimMontage* AttackMontage);
	void StopAttackMontage(UAnimMontage* AttackMontage);
	virtual int32 PlayDeathMontage();
	virtual void PlayDodgeMontage();

	UFUNCTION(BLueprintCallable)
	FVector GetTranslationWarpTarget();

	UFUNCTION(BLueprintCallable)
	FVector GetRotationWarpTarget();

	UFUNCTION(BLueprintCallable)
	virtual void AttackEnd();

	UFUNCTION(BLueprintCallable)
	virtual void DodgeEnd();

	UFUNCTION(BLueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	UPROPERTY(VisibleAnywhere)
	AWeapon* Equipped1hWeapon;

	UPROPERTY(VisibleAnywhere)
	AWeapon* Equipped2hWeapon;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* AttackMontage_1h;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* AttackMontage_2h;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAttributeComponent* Attributes;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere, Category = Combat)
	double WarpTargetDistance = 75.f;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDeathPose> DeathPose;

private:
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	int32 PlayRandomMontageSection(UAnimMontage* Montage);

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = Combat)
	UParticleSystem* HitParticles;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* DodgeMontage;

public:
	FORCEINLINE TEnumAsByte<EDeathPose> GetDeathPose() const { return DeathPose; }
};
