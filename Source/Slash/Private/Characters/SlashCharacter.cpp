// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GroomComponent.h"
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	 
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

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);

	ActionState = EActionState::EAS_HitReaction;
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

	Tags.Add(FName("EngageableTarget"));
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
	if (OverlappingWeapon)
	{
		EquipWeapon(OverlappingWeapon);
	}
}

void ASlashCharacter::Num1KeyPressed(const FInputActionValue& Value)
{
	if (CanDisarm1h())
	{
		Disarm();
	}
	else if (CanDisarm2h())
	{
		Disarm();
	}
	else if (CanArm1h())
	{
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		Arm();
	}
}

void ASlashCharacter::Num2KeyPressed(const FInputActionValue& Value)
{
	if (CanDisarm2h())
	{
		Disarm();
	}
	else if (CanDisarm1h())
	{
		Disarm();
	}
	else if (CanArm2h())
	{
		CharacterState = ECharacterState::ECS_EquippedTwoHandedWeapon;
		Arm();
	}
}

void ASlashCharacter::Attack(const FInputActionValue& Value)
{
	Super::Attack();

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

void ASlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	ActiveWeapon = Weapon;
	OverlappingItem = nullptr;

	FString WeaponType = Weapon->GetWeaponType();
	if (WeaponType == "One-Handed")
	{
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		Equipped1hWeapon = Weapon;
	}
	else if (WeaponType == "Two-Handed")
	{
		CharacterState = ECharacterState::ECS_EquippedTwoHandedWeapon;
		Equipped2hWeapon = Weapon;
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

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied && 
		CharacterState != ECharacterState::ECS_Unequipped;
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
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

void ASlashCharacter::Arm()
{
	ActionState = EActionState::EAS_EquippingWeapon;
	if (CharacterState == ECharacterState::ECS_EquippedOneHandedWeapon)
	{
		PlayEquipMontage(FName("Equip"), EquipMontage_1h);
	}
	else if (CharacterState == ECharacterState::ECS_EquippedTwoHandedWeapon)
	{
		PlayEquipMontage(FName("Equip"), EquipMontage_2h);
	}
}

void ASlashCharacter::Disarm()
{
	ActionState = EActionState::EAS_EquippingWeapon;
	if (CharacterState == ECharacterState::ECS_EquippedOneHandedWeapon)
	{
		PlayEquipMontage(FName("Unequip"), EquipMontage_1h);
	}
	else if (CharacterState == ECharacterState::ECS_EquippedTwoHandedWeapon)
	{
		PlayEquipMontage(FName("Unequip"), EquipMontage_2h);
	}
}

void ASlashCharacter::AttachWeaponToHand()
{
	if (CharacterState == ECharacterState::ECS_EquippedOneHandedWeapon)
	{
		Equipped1hWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		ActiveWeapon = Equipped1hWeapon;
	} 
	else if (CharacterState == ECharacterState::ECS_EquippedTwoHandedWeapon)
	{
		Equipped2hWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		ActiveWeapon = Equipped2hWeapon;
	}
}

void ASlashCharacter::AttachWeaponToScabbard()
{
	if (CharacterState == ECharacterState::ECS_EquippedOneHandedWeapon)
	{
		Equipped1hWeapon->Unequip(GetMesh(), FName("ThighSocket"));
	}
	else if (CharacterState == ECharacterState::ECS_EquippedTwoHandedWeapon)
	{
		Equipped2hWeapon->Unequip(GetMesh(), FName("NeckSocket"));
	}
	CharacterState = ECharacterState::ECS_Unequipped;
	ActiveWeapon = nullptr;
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}
