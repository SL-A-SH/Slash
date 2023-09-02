// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Health.h"
#include "Interfaces/PickupInterface.h"

void AHealth::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
	if (PickupInterface)
	{
		bool HealthAdded = PickupInterface->AddHealth(this);

		if (HealthAdded)
		{
			SpawnPickupSystem();
			SpawnPickupSound();

			Destroy();
		}
	}
}
