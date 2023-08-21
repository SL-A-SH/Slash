// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CharacterTypes.h"
#include "SlashCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UGroomComponent;
class AItem;
class UAnimMontage;
class AWeapon;

UCLASS()
class SLASH_API ASlashCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASlashCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BLueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

protected:
	virtual void BeginPlay() override;

	/**
	* Input actions
	*/
	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* SlashCharacterMappingContext;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* EKeyAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* Equip1hAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* Equip2hAction;

	/**
	* Input callbacks
	*/
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void EKeyPressed(const FInputActionValue& Value);
	void Num1KeyPressed(const FInputActionValue& Value);
	void Num2KeyPressed(const FInputActionValue& Value);
	void Attack(const FInputActionValue& Value);

	/**
	*	Play montage functions
	*/
	void PlayAttackMontage(UAnimMontage* AttackMontage);
	void PlayEquipMontage(const FName& SectionName, UAnimMontage* EquipMontage);

	UFUNCTION(BLueprintCallable)
	void AttackEnd();

	UFUNCTION(BLueprintCallable)
	void Arm1h();

	UFUNCTION(BLueprintCallable)
	void Arm2h();

	UFUNCTION(BLueprintCallable)
	void Disarm1h();

	UFUNCTION(BLueprintCallable)
	void Disarm2h();

	UFUNCTION(BLueprintCallable)
	void FinishEquipping();

	bool CanAttack();
	bool CanDisarm1h();
	bool CanDisarm2h();
	bool CanArm1h();
	bool CanArm2h();

private:
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* HeadHair;

	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Eyebrows;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(VisibleAnywhere)
	AWeapon* Equipped1hWeapon;

	UPROPERTY(VisibleAnywhere)
	AWeapon* Equipped2hWeapon;

	UPROPERTY(VisibleAnywhere)
	AWeapon* ActiveWeapon;

	/**
	* Animation Montages
	*/

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* AttackMontage_1h;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* AttackMontage_2h;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage_1h;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage_2h;

public:
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
};
