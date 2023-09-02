// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Health.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API AHealth : public AItem
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Health Properties")
	int32 HealthAmount;

public:
	FORCEINLINE int32 GetHealth() const { return HealthAmount; }
	FORCEINLINE void SetHealth(int32 PotionHealth) { this->HealthAmount = PotionHealth; }
};
