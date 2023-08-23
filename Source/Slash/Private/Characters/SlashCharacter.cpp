// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GroomComponent.h"
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "Components/BoxComponent.h"

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);

	HeadHair = CreateDefaultSubobject<UGroomComponent>(TEXT("HeadHair"));
	HeadHair->SetupAttachment(GetMesh());
	HeadHair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashCharacterMappingContext, 0);
		}
	}

	Tags.Add(FName("SlashCharacter"));
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();
	
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if (GetController())
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}
}

void ASlashCharacter::EKeyPressed(const FInputActionValue& Value)
{
	if (!OverlappingItem) return;

	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	FString WeaponType = OverlappingWeapon->GetWeaponType();
	if (OverlappingWeapon)
	{
		if (WeaponType == "One-Handed")
		{
			OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
			CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
			Equipped1hWeapon = OverlappingWeapon;
			ActiveWeapon = OverlappingWeapon;
		}
		else if (WeaponType == "Two-Handed")
		{
			OverlappingWeapon->Equip(GetMesh(), FName("DualHandedSocket"), this, this);
			CharacterState = ECharacterState::ECS_EquippedTwoHandedWeapon;
			Equipped2hWeapon = OverlappingWeapon;
			ActiveWeapon = OverlappingWeapon;
		}
		OverlappingItem = nullptr;
	}
}

void ASlashCharacter::Num1KeyPressed(const FInputActionValue& Value)
{
	if (CanDisarm1h())
	{
		CharacterState = ECharacterState::ECS_Unequipped;
		ActionState = EActionState::EAS_EquippingWeapon;
		PlayEquipMontage(FName("Unequip"), EquipMontage_1h);
	}
	else if (CanDisarm2h())
	{
		CharacterState = ECharacterState::ECS_Unequipped;
		ActionState = EActionState::EAS_EquippingWeapon;
		PlayEquipMontage(FName("Unequip"), EquipMontage_2h);
	}
	else if (CanArm1h())
	{
		PlayEquipMontage(FName("Equip"), EquipMontage_1h);
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		ActionState = EActionState::EAS_EquippingWeapon;
	}
}

void ASlashCharacter::Num2KeyPressed(const FInputActionValue& Value)
{
	if (CanDisarm2h())
	{
		CharacterState = ECharacterState::ECS_Unequipped;
		ActionState = EActionState::EAS_EquippingWeapon;
		PlayEquipMontage(FName("Unequip"), EquipMontage_2h);
	}
	else if (CanDisarm1h())
	{
		CharacterState = ECharacterState::ECS_Unequipped;
		ActionState = EActionState::EAS_EquippingWeapon;
		PlayEquipMontage(FName("Unequip"), EquipMontage_1h);
	}
	else if (CanArm2h())
	{
		PlayEquipMontage(FName("Equip"), EquipMontage_2h);
		CharacterState = ECharacterState::ECS_EquippedTwoHandedWeapon;
		ActionState = EActionState::EAS_EquippingWeapon;
	}
}

void ASlashCharacter::Attack(const FInputActionValue& Value)
{
	if (CanAttack())
	{
		if (ActiveWeapon->GetWeaponType() == "One-Handed")
		{
			PlayAttackMontage(AttackMontage_1h);
		}
		else if (ActiveWeapon->GetWeaponType() == "Two-Handed")
		{
			PlayAttackMontage(AttackMontage_2h);
		}

		ActionState = EActionState::EAS_Attacking;
	}
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied && 
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::CanDisarm1h()
{
	return ActionState == EActionState::EAS_Unoccupied && 
		CharacterState == ECharacterState::ECS_EquippedOneHandedWeapon
		&& Equipped1hWeapon;
}

bool ASlashCharacter::CanDisarm2h()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_EquippedTwoHandedWeapon
		&& Equipped2hWeapon;
}

bool ASlashCharacter::CanArm1h()
{
	return ActionState == EActionState::EAS_Unoccupied && 
		CharacterState == ECharacterState::ECS_Unequipped && 
		Equipped1hWeapon;
}

bool ASlashCharacter::CanArm2h()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped &&
		Equipped2hWeapon;
}

void ASlashCharacter::PlayAttackMontage(UAnimMontage* AttackMontage)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);

		const int32 Selection = FMath::RandRange(0, 1);
		FName SectionName = FName();

		switch (Selection)
		{
		case 0:
			SectionName = FName("Attack1");
			break;
		case 1:
			SectionName = FName("Attack2");
			break;
		default:
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
	}
}

void ASlashCharacter::PlayEquipMontage(const FName& SectionName, UAnimMontage* EquipMontage)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::Arm1h()
{
	if (Equipped1hWeapon)
	{
		Equipped1hWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		ActiveWeapon = Equipped1hWeapon;
	}
}

void ASlashCharacter::Arm2h()
{
	if (Equipped2hWeapon)
	{
		Equipped2hWeapon->Equip(GetMesh(), FName("DualHandedSocket"), this, this);
		ActiveWeapon = Equipped2hWeapon;
	}
}

void ASlashCharacter::Disarm1h()
{
	if (Equipped1hWeapon)
	{
		Equipped1hWeapon->Unequip(GetMesh(), FName("ThighSocket"));
	}
}

void ASlashCharacter::Disarm2h()
{
	if (Equipped2hWeapon)
	{
		Equipped2hWeapon->Unequip(GetMesh(), FName("NeckSocket"));
	}
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &ASlashCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(Equip1hAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Num1KeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack);
		EnhancedInputComponent->BindAction(Equip2hAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Num2KeyPressed);
	}
}

void ASlashCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (Equipped1hWeapon && Equipped1hWeapon->GetWeaponBox())
	{
		Equipped1hWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		Equipped1hWeapon->IgnoreActors.Empty();
	}

	if (Equipped2hWeapon && Equipped2hWeapon->GetWeaponBox())
	{
		Equipped2hWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		Equipped2hWeapon->IgnoreActors.Empty();
	}
}

