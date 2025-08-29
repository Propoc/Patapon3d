// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable CppUE4ProbableMemoryIssuesWithUObject
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Patapon_Main.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPatapon, Log, All);


UCLASS()

class PATAPON_API APatapon_Main : public ACharacter 
{
	GENERATED_BODY()

public:
	APatapon_Main();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorCtrl, AActor* DamageCauser);

public:	
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere , Category = "Debug")
	bool bDEBUG_LOG = true ;
    UPROPERTY(EditAnywhere , Category = "Debug")
    bool bDEBUG_LINES = true ;

	
	// Animation GetSets
	
	UFUNCTION(BlueprintPure)
	FString GetCommand();
	UFUNCTION(BlueprintPure)
	int GetState();
	UFUNCTION(BlueprintPure)
	FString GetStatus();
	UFUNCTION(BlueprintPure)
	bool GetPoisioned();
	UFUNCTION(BlueprintPure)
    bool GetCanAttack();
	UFUNCTION(BlueprintPure)
	bool GetChargedUp();
	UFUNCTION(BlueprintPure)
	bool GetFever();


	// Called from health to adjust behavior
	
	void ClearStatus();
	void ClearPoison();
	
	//Public stats needed for pawn mains
	
	UFUNCTION(BlueprintCallable)
	void ReceiveInput(int input);
	UFUNCTION(BlueprintCallable)
	void ReceiveCommand(int input);
		
	void SetFever(bool SetFever);
	
	
	APatapon_Main* Hatapon;
	FVector RelativeLocation;  // Relative location to Hatapon if not Hatapon;

	UFUNCTION(BlueprintCallable)
	void InitialSetup();
	
	UFUNCTION(BlueprintPure)
	AActor* GetTarget();

	UPROPERTY(EditAnywhere , Category = "Stats")
	FString PataponClass = "none";
	UPROPERTY(EditAnywhere , Category = "Stats")
	int Team = 1;

	
private:

	//Components
	class UHealth* HealthComponent;
	UCharacterMovementComponent* MovementComponent;
	class AAIController* AI;
	
	class UNiagaraComponent* BurnEffect;
	class UNiagaraComponent* PoisonEffect;
	
	//Stats
	UPROPERTY(EditAnywhere , Category = "Stats") // Range that can aquire the target
	float SpotRange = 2000;
	UPROPERTY(EditAnywhere , Category = "Stats") // Range that it can attack
	float AttackRange = 1000;
	UPROPERTY(EditAnywhere , Category = "Stats") // Ranged units should be retreat when this close
	float SafeRange = 300;
	UPROPERTY(EditAnywhere , Category = "Stats") // Movement Component Speed
	float Speed = 600;
	UPROPERTY(EditAnywhere , Category = "Stats") // Rotation Speed
	float RotationSpeed = 2;
	UPROPERTY(EditAnywhere , Category = "Stats") // Charged or Fevered in mult ratio
	float DamageBoostL1 = 1.5;
	UPROPERTY(EditAnywhere , Category = "Stats") // Charged and Fevered in mult ratio
	float DamageBoostL2 = 2;
	UPROPERTY(EditAnywhere , Category = "Stats") // Simple Defend (Additive bonus to 0 resistance diminishing returns on already resistant characters)
	float DefendBoostL1 = 20;
	UPROPERTY(EditAnywhere , Category = "Stats") // Defend with charge or fever
	float DefendBoostL2 = 40;
	UPROPERTY(EditAnywhere , Category = "Stats") // Defend with charge and fever
	float DefendBoostL3 = 60;
	UPROPERTY(EditAnywhere , Category = "Stats") // Simple Additive
	float ResistanceBonusL1 = 20;
	UPROPERTY(EditAnywhere , Category = "Stats") // Simple Additive
	float ResistanceBonusL2 = 40;
	UPROPERTY(EditAnywhere , Category = "Stats") // Simple Additive
	float ResistanceBonusL3 = 60;
	


	UPROPERTY(EditAnywhere , Category = "Stats")
	bool BossPriority = false;

	
	//Props Items
	UPROPERTY(EditAnywhere, Category = "Items")  //"none" "ranged" "melee"
	FString AttackType = "none";             
	UPROPERTY(EditAnywhere, Category = "Items") //Weapon class which is being used actively, attack type determines what to do with it
	TSubclassOf<class AWeapon> WeaponClass;    
	UPROPERTY(EditAnywhere, Category = "Items") //Visual weapon when holding no collision
	TSubclassOf<class AWeapon> LeftHandProp;   
	UPROPERTY(EditAnywhere, Category = "Items") //Visual weapon when holding no collision
	TSubclassOf<class AWeapon> RightHandProp;  
	UPROPERTY(EditAnywhere, Category = "Items") // Which weapon is main weapon 
	FString WeaponHand = "right";	

	// Pointerlar lazım çünkü spawnlanması gerekiyor
	UPROPERTY(VisibleAnywhere , Category = "Items")
	AWeapon* LeftHandWeapon;
	UPROPERTY(VisibleAnywhere , Category = "Items")
	AWeapon* RightHandWeapon;
	UPROPERTY(VisibleAnywhere , Category = "Items")
	AWeapon* MainWeapon;
	

	// Pointers at beginplay
	UPROPERTY(VisibleAnywhere)
	AActor* Target;
	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> PossibleTargets;
	AActor* Boss;
	
	// Orders and behaviour
	int state = 0;
	FString command = "none";
	FString status = "none";
	bool fever = false;
		
	bool IsMoving = false;
	bool CanAttack = false;
	bool charged = false;


	FVector Anchor; //Anchor for retreat point
	void AcquireTarget();
	void AttackCheck();
	void TickCheck();
	void EndAnimationCheck();
	void EndCycleCheck();
	
	void Move(FVector MoveTo, FString command = "none",float speedmultiple = 1);
	void Halt(bool resetcommand = false);
	void AttackStart();
	UFUNCTION(BlueprintCallable)
	void Attack();
	void Charge();
	void Defend();
	void Retreat();
	UFUNCTION(Blueprintcallable)
	void RetreatEnded();
	void Hop();
	void Dance();
	
	
};




