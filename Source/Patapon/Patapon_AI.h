// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Patapon_AI.generated.h"


UCLASS()
class PATAPON_API APatapon_AI : public AAIController
{
	GENERATED_BODY()
	
public:
	//APatapon_AI(const FObjectInitializer& ObjectInitializer);
	
	virtual void Tick(float DeltaTime) override;

	
protected:
	virtual void BeginPlay() override;
	//virtual void OnPossess(APawn* InPawn) override;

};
