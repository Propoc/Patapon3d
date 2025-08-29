
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Health.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogHealth, Log, All);



UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Normal,
	Crit,
	Burn,
	Poison,
	Fatal
};


UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent)  )
class PATAPON_API UHealth : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealth();

	UFUNCTION(BlueprintCallable)
	void ProcessDamage(float Damage , AActor* DamageCauser);
	
	UFUNCTION(BlueprintImplementableEvent)
	void DamageToBlueprint(float damage , EDamageType Type);
	UFUNCTION(BlueprintImplementableEvent)
	void ApplyKnockbackBlueprint(FVector Hitfrom);

	
	void ResetResistance();
	void AmplifyResistance(float ratio, float status_ratio);
	
	FString StatusEffect(class AWeapon* Weapon);
	bool PoisonCheck(AActor* DamageCauser);
	void ClearStatusEffect();
	void ClearPoisonEffect();
	
	UFUNCTION(BlueprintCallable)
	void Kill();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY(EditAnywhere , Category = "Debug")
	bool bDEBUG_LOG = true ;
	
	// Stats
	UPROPERTY(EditAnywhere , BlueprintReadWrite , Category = "Attributes")
	float hp;
	UPROPERTY(EditAnywhere , BlueprintReadWrite , Category = "Attributes")
	float maxhp=100;


	bool poisoned = false;  //POISON HERE FOR NOW FOR DEBUGGING IN EDITOR
	
private:
	
	AActor* Owner;
	
	UPROPERTY(EditAnywhere , Category = "Attributes") // What ratio of damage is taken after all calcululations 100 is totally immune to everything
	float Resistance = 0;
	UPROPERTY(EditAnywhere , Category = "Attributes") // Percent resist for all (-1 means immune) 
	float CritRst = 0;
	UPROPERTY(EditAnywhere , Category = "Attributes") // Cannot be lower than base damage recieved *(WeaponCritDamageMultipication-ThisValue)
	float CritDmgRst = 0;
	UPROPERTY(EditAnywhere , Category = "Attributes")
	float KnockbackRst = 0;
	UPROPERTY(EditAnywhere , Category = "Attributes")
	float StunRst = 0;
	UPROPERTY(EditAnywhere , Category = "Attributes")
	float SleepRst = 0;
	UPROPERTY(EditAnywhere , Category = "Attributes")
	float BurnRst = 0;
	UPROPERTY(EditAnywhere , Category = "Attributes")
	float FreezeRst = 0;
	UPROPERTY(EditAnywhere , Category = "Attributes")
	float PoisionRst = 0;


	// Buffers to return to normal self when bonuses perish
	float ResistanceActive;
	float StatusResistanceBonus;

	// Status
	float poisonduration = 0;
	float poisondamage = 0;
	float poisontick = 2;

	
	float burndamage = 0;
	float burntick = 1;
	
	
	FTimerHandle StatusTimer;
	FTimerHandle PoisonTimer;
	FTimerHandle BurnTimer;
	
	void PoisionTick();
	void BurnTick();


};
