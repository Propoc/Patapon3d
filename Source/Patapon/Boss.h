
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Boss.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogBoss, Log, All);

UCLASS()
class PATAPON_API ABoss : public ACharacter 
{
	GENERATED_BODY()

public:
	ABoss();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorCtrl, AActor* DamageCauser);

public:	
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere , Category = "Debug")
	bool bDEBUG_LOG = true ;
	UPROPERTY(EditAnywhere , Category = "Debug")  // Always select this attack, none for cancel
	FString DEBUG_ATTACK = "none" ;
	UPROPERTY(EditAnywhere , Category = "Debug")  // Always select this attack, none for cancel
	bool bDEBUG_RANGE = true ;
	UPROPERTY(EditAnywhere  ,Category = "Debug")
	bool bStartIdle = false;
	
	UStaticMeshComponent* Sphere;
	FVector Weakpoint;

	UPROPERTY(EditAnywhere, Category = "Items") 
	TSubclassOf<class AWeapon> HeadButtClass;
	UPROPERTY(VisibleAnywhere , Category = "Items")
	UStaticMeshComponent* HeadButtArea;
	UPROPERTY(VisibleAnywhere , Category = "Items")
	AWeapon* HeadButt;
	
	UPROPERTY(EditAnywhere  ,Category = "Stats")
	float detectRange = 5000;
	UPROPERTY(EditAnywhere  ,Category = "Stats")
	float attackRange = 2500;
	
	
	UPROPERTY(BlueprintReadWrite)
	FString status = "none";
	UPROPERTY(BlueprintReadWrite)
	FString attacktype = "none";


	
	UPROPERTY(EditAnywhere , Category = "Items")
	TSubclassOf<class AWeapon> BreatheClass;
	UPROPERTY(VisibleAnywhere , Category = "Items")
	AWeapon* Breathe;

	void CycleInfo(int CommandCounter);

	// Animation Triggers
	UFUNCTION(BlueprintCallable)
	void ApplyDamage(AActor* whom);
	UFUNCTION(BlueprintCallable)
	void SetBreathe(bool bSet);
	
	
	TArray<class APatapon_Main*> Patapons;
	
private:

	class UHealth* HealthComponent;
	UCharacterMovementComponent* MovementComponent;
	class AAIController* AI;


	class APatapon_Main* GetClosestPatapon();
	void SetupTargets();

	class APatapon_Main* Hatapon;
	float DistanceToHatapon;


	void Move(class APatapon_Main* Target);
	void Back(FVector To);
	
	UPROPERTY(VisibleAnywhere  ,Category = "Stats")
	float AttackWeight = 0;
	UPROPERTY(VisibleAnywhere  ,Category = "Stats")
	float MoveWeight = 0;
	UPROPERTY(VisibleAnywhere  ,Category = "Stats")
	float IdleWeight = 0;

	
	bool bAttackReady = false;

	int CycleWithoutAttack =  0;
	int CycleWithoutAttackSafe = 0;
	int CycleAttackTimingSafe = 0;
	int CycleCooldown = 0;

	bool bBreathing = false;
	int BreatheGap = 2;
	
};
