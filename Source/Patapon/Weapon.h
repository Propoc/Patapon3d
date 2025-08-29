
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWeapon, Log, All);

UCLASS()
class PATAPON_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();

protected:
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere , Category = "Debug")
	bool bDEBUG_LOG = true ;
	
	UPROPERTY(BlueprintReadWrite,VisibleAnywhere, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovementComp;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* WeaponMesh;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* DefaultSceneRoot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class UNiagaraComponent* NiagaraComponent;


	UFUNCTION(BlueprintCallable)
	void StartAnimation(int type);
	UFUNCTION(BlueprintCallable)
	void StopAnimation();

	UFUNCTION(BlueprintCallable)
	int GetAnimationData();
	
	UFUNCTION(BlueprintCallable)
	void Melee(AActor* Target);
	UFUNCTION(BlueprintCallable)
	void Ranged(AActor* Target);
	UFUNCTION(BlueprintCallable)
	void RangedL(FVector Location);

	UPROPERTY(VisibleAnywhere , Category ="Stats")  //Percentage based owner boosts this at spawn
	float DamageBuff=0; 
	UPROPERTY(EditAnywhere , Category ="Stats")
	float MinDamage=20;
	UPROPERTY(EditAnywhere , Category ="Stats")
	float MaxDamage=30;

	UPROPERTY(EditAnywhere,  Category ="Stats")
	float CritCnc = 0;
	UPROPERTY(EditAnywhere,  Category ="Stats")
	float CritDmgMultiplier = 200;
	UPROPERTY(EditAnywhere,  Category ="Stats")
	float KnockbackCnc = 0;
	UPROPERTY(EditAnywhere,  Category ="Stats")
	float SleepCnc = 0;
	UPROPERTY(EditAnywhere,  Category ="Stats")
	float StunCnc = 0;
	UPROPERTY(EditAnywhere,  Category ="Stats")
	float BurnCnc = 0;
	UPROPERTY(EditAnywhere,  Category ="Stats")
	float FreezeCnc = 0;
	UPROPERTY(EditAnywhere,  Category ="Stats")
	float PoisionCnc = 0;

	UPROPERTY(EditAnywhere,  Category ="Stats")
	float PoisonDamage = 0;
	UPROPERTY(EditAnywhere,  Category ="Stats")  //Tick occurs at 1 sec
	float PoisonDuration = 0;
	UPROPERTY(EditAnywhere,  Category ="Stats")
	float BurnDamage = 0;
	UPROPERTY(EditAnywhere,  Category ="Stats")  //Tick occurs at 0.5 sec
	float BurnDuration = 0;

	UPROPERTY(EditAnywhere , Category ="Stats-Ranged") // Randomness circle radii
	float Accuracy=75;
	UPROPERTY(EditAnywhere , Category ="Stats-Ranged") // Point shot , overrides accuracy
	bool Deadeye=true;
	UPROPERTY(EditAnywhere,  Category ="Stats-Ranged")
	float ProjectileSpeed=1000;
	
	
private:
	
	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult );

	FTimerHandle DeathTimer;
	UPROPERTY(EditAnywhere)
	float lifetime = 5;
	void Kill();

	int attacktype = 0;
	
};


