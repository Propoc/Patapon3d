// Fill out your copyright notice in the Description page of Project Settings.

#include "Health.h"

#include "Boss.h"
#include "Patapon_Main.h"
#include "Pawn_Main.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogHealth);
#define LOG_OBJ(LogCategory, Obj, Format, ...) \
if (Obj && Obj->bDEBUG_LOG) { \
FString DisplayName = UKismetSystemLibrary::GetDisplayName(Obj->GetOwner()); \
UE_LOG(LogCategory, Log, TEXT("[%s] " Format), \
*DisplayName, ##__VA_ARGS__); \
}


class UIntProperty;

UHealth::UHealth()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UHealth::BeginPlay()
{
	Super::BeginPlay();
	
	Owner = GetOwner();
	hp = maxhp;
	ResistanceActive = Resistance;
	
}


void UHealth::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UHealth::ProcessDamage(float Damage , AActor* DamageCauser)  //All Damage stuff calculates here, character notifies this
{
	if (Resistance==100){LOG_OBJ(LogHealth, this, TEXT(" negated all damage - is immune")); return;}

	if (DamageCauser == Owner)
	{
		hp-=Damage;
		DamageToBlueprint(Damage,EDamageType::Normal);
	}

	else
	{
		AWeapon* Weapon = Cast<AWeapon>(DamageCauser);   
		bool didCrit = false;
		float DamageToApply = 0;

		if (CritRst != -1)
		{
			didCrit = UKismetMathLibrary::RandomBoolWithWeight((Weapon->CritCnc-CritRst)/100);
		}
		if (CritDmgRst > Weapon->CritDmgMultiplier-100) CritDmgRst = Weapon->CritDmgMultiplier-100;
		if (didCrit) DamageToApply = Damage * (Weapon->CritDmgMultiplier-CritDmgRst)/100 ;
		else DamageToApply = Damage;
		DamageToApply = DamageToApply - DamageToApply * ResistanceActive/100;
		hp -= DamageToApply;
		
		LOG_OBJ(LogHealth,this,TEXT("  got hit by %s for %f , crited ? %d  ::: Raw damage was %f, Crit ratio was %f, Resistance was %f ") ,*FString(UKismetSystemLibrary::GetDisplayName(DamageCauser)),DamageToApply , didCrit, Damage, (Weapon->CritDmgMultiplier-CritDmgRst)/100 , Resistance);

		if (didCrit){DamageToBlueprint(DamageToApply,EDamageType::Crit);}
		else{DamageToBlueprint(DamageToApply,EDamageType::Normal);}

	}

	if (hp<=0)
	{
		hp=0;
		Kill();
	}
}


void UHealth::ResetResistance()
{
	ResistanceActive = Resistance;

	CritRst -= StatusResistanceBonus;
	KnockbackRst -= StatusResistanceBonus;
	StunRst -= StatusResistanceBonus;
	SleepRst -= StatusResistanceBonus;
	BurnRst -= StatusResistanceBonus;
	FreezeRst -= StatusResistanceBonus;
	PoisionRst -= StatusResistanceBonus;
	StatusResistanceBonus = 0;
}

void UHealth::AmplifyResistance(float ratio , float status_ratio)  // İlginç yaptım ne kadar fazlaysa o kadar az etklili
{
	if( Resistance == 100) { return;}
	
	ResistanceActive = Resistance + ratio * ( 100 - Resistance)/100;
	if(ResistanceActive >= 90){ ResistanceActive = 90; }

	StatusResistanceBonus = status_ratio;
	CritRst+= StatusResistanceBonus;
	KnockbackRst += StatusResistanceBonus;
	StunRst += StatusResistanceBonus;
	SleepRst += StatusResistanceBonus;
	BurnRst += StatusResistanceBonus;
	FreezeRst += StatusResistanceBonus;
	PoisionRst += StatusResistanceBonus;
	
}



// Chance - Resist now maybe later Chance % Resist

// Knockback calculates in BP for now , burn is different from others it should change movement 

FString UHealth::StatusEffect(AWeapon* Weapon)   //This is called from character after damage and returns if something triggers*  
{
	if (KnockbackRst != -1)
	{
		if (UKismetMathLibrary::RandomBoolWithWeight((Weapon->KnockbackCnc-KnockbackRst)/100))
		{
			ApplyKnockbackBlueprint(Weapon->GetOwner()->GetActorLocation());
			LOG_OBJ(LogHealth, this, TEXT(" gets knocked down, (Cnc=%f / Rst=%f) "), Weapon->KnockbackCnc,KnockbackRst);
			Owner->GetWorldTimerManager().SetTimer(StatusTimer,this,&UHealth::ClearStatusEffect,2);
			return "knockback";
		}
	}
	
	if (StunRst != -1)
	{
		if (UKismetMathLibrary::RandomBoolWithWeight((Weapon->StunCnc-StunRst)/100))
		{
			LOG_OBJ(LogHealth, this, TEXT(" gets stunned, (Cnc=%f / Rst=%f) "), Weapon->StunCnc,StunRst);
			Owner->GetWorldTimerManager().SetTimer(StatusTimer,this,&UHealth::ClearStatusEffect,1);
			return "stun";
		}
	}
	
	if (SleepRst != -1)
	{
		if (UKismetMathLibrary::RandomBoolWithWeight((Weapon->SleepCnc-SleepRst)/100))
		{
			LOG_OBJ(LogHealth, this, TEXT(" gets sleept, (Cnc=%f / Rst=%f) "), Weapon->SleepCnc,SleepRst);
			Owner->GetWorldTimerManager().SetTimer(StatusTimer,this,&UHealth::ClearStatusEffect,4);
			return "sleep";
		}
	}
	
	if (FreezeRst != -1)
	{
		if (UKismetMathLibrary::RandomBoolWithWeight((Weapon->FreezeCnc-FreezeRst)/100))
		{
			LOG_OBJ(LogHealth, this, TEXT(" gets frozen, (Cnc=%f / Rst=%f) "),Weapon->FreezeCnc,FreezeRst);
			Owner->GetWorldTimerManager().SetTimer(StatusTimer,this,&UHealth::ClearStatusEffect,3);
			return "freeze"; 
		}
	}
	
	if (BurnRst != -1)  
	{
		if(UKismetMathLibrary::RandomBoolWithWeight((Weapon->BurnCnc-BurnRst)/100))
		{
			if(Weapon->BurnDuration == 0 || Weapon->BurnDamage == 0)  
			{
				LOG_OBJ(LogHealth, this, TEXT(" got butrned, but burn attributes are not set on the weapon that got hit with it ??? " ));
				return "none";
			}
			
			
			LOG_OBJ(LogHealth, this, TEXT(" gets burned , (Cnc=%f / Rst=%f) "),Weapon->BurnCnc,BurnRst);
			Owner->GetWorldTimerManager().SetTimer(StatusTimer,this,&UHealth::ClearStatusEffect, Weapon->BurnDuration);
			
			burndamage = Weapon->BurnDamage;
			Owner->GetWorldTimerManager().SetTimer(BurnTimer,this,&UHealth::BurnTick,burntick,true);
			return "burn";
		}
	}

	return "none";
}

bool UHealth::PoisonCheck(AActor* DamageCauser) // Called just after the status
{
	AWeapon* Weapon = Cast<AWeapon>(DamageCauser);   
	
	if (PoisionRst != -1) 
	{
		if(UKismetMathLibrary::RandomBoolWithWeight((Weapon->PoisionCnc-PoisionRst)/100)) 
		{
			if(Weapon->PoisonDuration == 0 || Weapon->PoisonDamage == 0)  
			{
				LOG_OBJ(LogHealth, this, TEXT(" got poisoned, but poision attributes are not set on the weapon that got hit with it ??? " ));
				return false;
			}
			
			if (!poisoned)
			{
				poisonduration = Weapon->PoisonDuration;
				poisondamage = Weapon->PoisonDamage;
				poisoned = true;
				Owner->GetWorldTimerManager().SetTimer(PoisonTimer,this,&UHealth::PoisionTick,poisontick,true);
				LOG_OBJ(LogHealth, this, TEXT(" got poisoned, (Cnc=%f / Rst=%f) " ) , Weapon->PoisionCnc,PoisionRst);
			}
			else
			{
				poisonduration = (Weapon->PoisonDuration>poisonduration) ? Weapon->PoisonDuration : poisonduration;
				poisondamage = (Weapon->PoisonDamage>poisondamage) ? Weapon->PoisonDamage : poisondamage;
				LOG_OBJ(LogHealth, this, TEXT(" poison attributes renewed, (Cnc=%f / Rst=%f) ") , Weapon->PoisionCnc,PoisionRst);
			}

			return true;
		}
		return false;
	}
	return false;
	
}

void UHealth::ClearStatusEffect()  // this is called after a time  //DEĞİŞİECEK SADECE PATAPONA KAYITILI
{
	Owner->GetWorldTimerManager().ClearTimer(StatusTimer);
	Cast<ACharacter>(Owner)->GetCapsuleComponent()->SetSimulatePhysics(false); //DEĞİŞ
	Cast<APatapon_Main>(Owner)->ClearStatus();
	
	Owner->GetWorldTimerManager().ClearTimer(BurnTimer);
	burndamage = 0;
}

void UHealth::ClearPoisonEffect()
{
	poisoned=false;
	poisondamage = 0;
	Owner->GetWorldTimerManager().ClearTimer(PoisonTimer);

		
	Cast<APatapon_Main>(Owner)->ClearPoison();   //Again patapon spesific problem
}


void UHealth::PoisionTick() //Poison is calculated indivdiually
{
	LOG_OBJ(LogHealth, this, TEXT(" %f damage from poison tick"), poisondamage);
	ProcessDamage(poisondamage,Owner);


	poisonduration -= 1;
	if ( poisonduration <= 0)
	{
		ClearPoisonEffect();
	}
}
void UHealth::BurnTick() //Burn is calculated indivdiually  unlike poision it ends one a set time will look into  it
{
	LOG_OBJ(LogHealth, this, TEXT(" took %f damage from burn tick"), burndamage);
	ProcessDamage(burndamage,Owner);
}


void UHealth::Kill()  //Death triggers array change in pawn  !!!* Also notify other entities in case someone targets this
{
	LOG_OBJ(LogHealth, this, TEXT(" got killed"));

	AActor* Pawn = UGameplayStatics::GetActorOfClass(GetWorld(),APawn_Main::StaticClass());
	APawn_Main* pwn = Cast<APawn_Main>(Pawn);
	pwn->Patapons.Remove(Cast<APatapon_Main>(Owner));

	AActor* Boss = UGameplayStatics::GetActorOfClass(GetWorld(),ABoss::StaticClass());
	ABoss* bss = Cast<ABoss>(Boss);
	bss->Patapons.Remove(Cast<APatapon_Main>(Owner));


	
	TArray<AActor*> Attached;
	Owner->GetAttachedActors(Attached);
	for (AActor* Attach : Attached)
	{
		Attach->Destroy();
	}
	Owner->Destroy();
	
}

