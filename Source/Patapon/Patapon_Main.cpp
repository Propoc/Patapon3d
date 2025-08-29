#include "Patapon_Main.h"
#include "Boss.h"

#include "Health.h"
#include "NiagaraComponent.h"
#include "Patapon_AI.h"
#include "Weapon.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"



DEFINE_LOG_CATEGORY(LogPatapon);
#define LOG_OBJ(LogCategory,Obj, Format, ...) \
if (Obj && Obj->bDEBUG_LOG) { \
FString DisplayName = UKismetSystemLibrary::GetDisplayName(Obj); \
UE_LOG(LogCategory, Warning, TEXT("[%s] " Format), \
*DisplayName, ##__VA_ARGS__); \
}

// Enhanced log with class/object name
//LOG_OBJ(LogPatapon, this, TEXT("Actor is at %s"), *GetActorLocation().ToString());

APatapon_Main::APatapon_Main()
{
	PrimaryActorTick.bCanEverTick = false;
}


void APatapon_Main::BeginPlay()
{
	Super::BeginPlay();
	
	//Get essential components 
	MovementComponent = GetCharacterMovement();
	HealthComponent = FindComponentByClass<UHealth>();
	BurnEffect = Cast<UNiagaraComponent>(GetDefaultSubobjectByName(TEXT("BurnEffect")));
	PoisonEffect = Cast<UNiagaraComponent>(GetDefaultSubobjectByName(TEXT("PoisonEffect")));
	
	OnTakeAnyDamage.AddDynamic(this, &APatapon_Main::ReceiveDamage);
	

	AI = Cast<AAIController>(GetController());
	
	//Setup props
	GetMesh()->HideBoneByName(TEXT("LeftProp"),EPhysBodyOp::PBO_None);
	GetMesh()->HideBoneByName(TEXT("RightProp"),EPhysBodyOp::PBO_None);
	
	if (LeftHandProp)
	{
		LeftHandWeapon = GetWorld()->SpawnActor<AWeapon>(LeftHandProp);
		LeftHandWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("L_Socket"));
		LeftHandWeapon->SetOwner(this);
		LeftHandWeapon->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		LeftHandWeapon->ProjectileMovementComp->bSimulationEnabled=false;
	}

	if (RightHandProp)
	{
		RightHandWeapon = GetWorld()->SpawnActor<AWeapon>(RightHandProp);
		RightHandWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("R_Socket"));
		RightHandWeapon->SetOwner(this);
		RightHandWeapon->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RightHandWeapon->ProjectileMovementComp->bSimulationEnabled=false;
	}

	if (WeaponHand == "right")
	{
		MainWeapon = RightHandWeapon;
	}
	else if (WeaponHand == "left")
	{
		MainWeapon = LeftHandWeapon;
	}

	MovementComponent->MaxWalkSpeed = Speed;
}

void APatapon_Main::InitialSetup() // Tüm düşmanların arrayi her frame aramasın diye, değişmesi lazım !* Boss varsa otomatik focus
{
	TArray<AActor*> AllTargets;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), StaticClass(), AllTargets);
	
	for (AActor* MaybeTarget : AllTargets)
	{
		APatapon_Main* PossibleEnemy = Cast<APatapon_Main>(MaybeTarget);
		if (PossibleEnemy->Team != Team)
		{
			PossibleTargets.Add(MaybeTarget);
		}
		
	}
	Boss = UGameplayStatics::GetActorOfClass(GetWorld(), ABoss::StaticClass());
	if (Boss)
	{
		BossPriority = true;
	}
}

void APatapon_Main::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);
	if (PataponClass == "Hata")  // For others this variable is relative position initiated at startup for movement and placement to Hatapon , For Hatapon its own location real time
	{
		RelativeLocation = GetActorLocation();
	}
	TickCheck();


	//Rotational Checks

	if (PataponClass == "Hata") return;
	if (status != "none") return;
	if (!Hatapon) return;  //Hatapon does not get set instantly
	
	if (!Target) AcquireTarget();

	if (!Target) 
	{
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), Hatapon->GetActorRotation(), DeltaTime, RotationSpeed);
		SetActorRotation(NewRotation);
		return;
	}

	if (command != "move" || command != "recover" || command != "retreat")
	{
		
		FRotator CurrentRotation = GetActorRotation();
		FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
		ToTarget.Z = 0;
	
		if (ToTarget.IsNearlyZero()) return;
		FRotator DesiredRotation = ToTarget.Rotation();
	
		DesiredRotation.Pitch = 0.0f;
		DesiredRotation.Roll = 0.0f;

		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, DesiredRotation, DeltaTime, RotationSpeed);

		SetActorRotation(NewRotation);
	}


}

void APatapon_Main::TickCheck()  // Her Frame Checklenmesi gereken bilgiler
{
	

	if (status == "burn")    // Yanıyorsa random koşması lazım değişecek !!!!*
	{
		FVector Normal = Boss->GetActorLocation()-GetActorLocation();
		Normal.Normalize();
		FVector NextStep = GetActorLocation() - Normal * 200;
		Move(NextStep,"none",1.4);

		return;
	}

	
	if (command == "recover")   // Recovering after the retreat 
	{

		FVector NextStep = Anchor;
		Move(NextStep,"none",3);

		if ( FVector::Dist(GetActorLocation(),Anchor) <= 200)
		{
			Halt(true);
		}
		return;
	}

	if (command == "reset")  // Recovering from status
	{
		
		FVector NextStep = Hatapon->GetActorLocation()+RelativeLocation+Hatapon->GetActorForwardVector()*200;
		Move(NextStep,"none",2);
		
		if (FVector::Dist(GetActorLocation(),Hatapon->GetActorLocation()+RelativeLocation)<=200)
		{
			state=0;
			Halt(true);
		}
		return;
	}
	
	
	if (CanAttack)  // Attack always checks its depended on tick factors
	{
		AttackCheck();
		
		if (command == "attackpending")  // This is just a transition state, a "first hit delay" just so everyone does not attack all at one
		{
			if (UKismetMathLibrary::RandomBoolWithWeight(0.1)) // Ugly binomial chance generator should be gone !!
			{
				AttackStart();
			}
		}
	}
	
	if (command == "move")   //Move Tickte çünkü hatapondan yolunu bulucak direk olacağı yere gitmesin ai controllerden hataponu izleyerek değişmesi gerekiyor !!!!!!!!*
	{
		if (PataponClass == "Hata")
		{
			FVector NextStep = GetActorLocation()+GetActorForwardVector()*200;

			// Frontline bi de pay bırak
			if (FVector::Dist(NextStep,Boss->GetActorLocation()) <= 3600)  
			{
				Move(GetActorLocation());
			}
			else
			{
				Move(NextStep);
			}
		}
		else
		{
			FVector NextStep = Hatapon->GetActorLocation()+RelativeLocation+Hatapon->GetActorForwardVector()*200;
			Move(NextStep);
		}
	}

	else if (command == "retreat")  // Same reason retreat is in here maybe change move to continous move
	{
		FVector Normal = Boss->GetActorLocation()-GetActorLocation();
		Normal.Normalize();
		FVector NextStep = GetActorLocation() - Normal * 200;
		Move(NextStep,"none",3);
	
	}

}

void APatapon_Main::EndAnimationCheck() // Called in ABP --- by attack defend charge jump dance with charged variants --- others handled differently , some orders need recalculation after animation ends
{
	if (command == "none") {return;}  // If already is in netural
		
	if (CanAttack)  // Check if it can attack again in cycle (if attacking very fast)  !!!!* not right now
	{
		CanAttack = false;
		charged = false;
		Halt(true);
	}
	
	if (command == "defend")
	{
		HealthComponent->ResetResistance();
		charged = false;
		Halt(true);
	}
	
	else
	{
		Halt(true);
	}
}

void APatapon_Main::EndCycleCheck()  // This is called on Pawn for command ended
{
	if (command == "move")
    {
    	Halt(true);
     } 
    		
    if (command == "pursue" || command == "attackpending")  // if still pursuing or waiting initial hit give up  maybe slowwwwwly come back to the anchor
    {
    	CanAttack = false;
    	Halt(true);
    }
	
}


void APatapon_Main::AttackCheck()  // Calls first in tick when attacking , Should attack, close distance 
{
	if (command == "attack" || command == "attackpending") return;  //Already in Attack animation or waiting
	if (!Target) return;  //No target

	if (AttackType == "ranged")
	{
		FVector TargetLocation = Target->GetActorLocation();
		TargetLocation.Z = GetActorLocation().Z;
		
		if (FVector::Dist(GetActorLocation(), TargetLocation) <= SafeRange )
		{
			FVector v = TargetLocation-GetActorLocation();
			v.Normalize();
			Move(GetActorLocation()-v*SafeRange,"pursue",2);
		} 
		else if (FVector::Dist(GetActorLocation(), TargetLocation) <= AttackRange)  
		{
			Halt(true);
			command = "attackpending";
		}
		else
		{
			Move(TargetLocation,"pursue",2);
		}
	}

	else if (AttackType == "melee")
	{
		FHitResult Hit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		FVector TargetLocation = Cast<ABoss>(Target)->Weakpoint;
		TargetLocation.Z = GetActorLocation().Z;
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), TargetLocation, ECC_Visibility, QueryParams);
		
		FColor LineColor = bHit ? FColor::Red : FColor::Green;

		if(bDEBUG_LINES)  DrawDebugLine(GetWorld(),GetActorLocation(),TargetLocation,LineColor,false,2.0f,0,2.0f );

		if (FVector::Dist(GetActorLocation(), Hit.Location) <= AttackRange )
		{
			Halt(true);
			command = "attackpending";
		}
		else
		{
			Move(Hit.Location,"pursue",2);
		}
	}
}



void APatapon_Main::AttackStart() // This is the trigger for attack animation
{
	command = "attack";

	if (RightHandWeapon && LeftHandWeapon)		// These notify the weapon to start with simultaniously if everything goes in order they stop themselves 
	{
		
		if (fever || charged)
		{
			LeftHandWeapon->StartAnimation(2);
			RightHandWeapon->StartAnimation(2);
		}
		else
		{
			LeftHandWeapon->StartAnimation(1);
			RightHandWeapon->StartAnimation(1);	
		}
	}
}


void APatapon_Main::Move(FVector MoveTo, FString changeCommand ,float speedmultiple)  //Start moving to target
{
	if (changeCommand != "none") {command = changeCommand;}

	if(bDEBUG_LINES) DrawDebugSolidBox(GetWorld(), MoveTo, FVector(30, 30, 30), FColor::Green, false, 0, 0);
	IsMoving = true;
	MovementComponent->MaxWalkSpeed = Speed*speedmultiple;
	AI->MoveToLocation(MoveTo,0.1);
}


void APatapon_Main::Halt(bool resetcommand )   //Stop moving set speed to default
{
	if (resetcommand){command = "none";}
	
	IsMoving = false;
	MovementComponent->StopActiveMovement();
	MovementComponent->MaxWalkSpeed = Speed;
}



// Ipnout recieved here
// ABP watches state command and status on every tick no need to notify, can be changed to trigger when these change only ?* not very effective even if it is done so leave it now 
// Where magic happens here called from Pawn these function are.

void APatapon_Main::ReceiveInput(int input)
{
	state = input;
}

void APatapon_Main::ReceiveCommand(int input)
{
	
	if (input == -9) {charged = false; return;}  //Missed first combo

	
	//Before Checks
	state = 0;
	if (status != "none" && input !=6)   //Command refused when taken order ?* will change to resume when its viable
	{
		LOG_OBJ(LogPatapon, this, TEXT("has status, cannot take command"));
		return;
	}

	// If not compatable after charge reset the charge
	if (input >= 0)  
	{
		if (charged && input !=1 && input !=2)
		{
			charged = false;
		}
	}

	
	// Safeguard check if it is doing a command when a viable order recieved while recovering from buring or so (Move beat animations needed)
	if (input >= 0 && command != "none") Halt(true);


	/////////////////////
	
	switch (input)
	{
	case -3:   // Called at input error or failed order  - Resets to idle first then state to -1 , full reset to default condition
		Halt(true);
		CanAttack = false;
		charged = false;
		if (LeftHandWeapon && RightHandWeapon){LeftHandWeapon->StopAnimation();RightHandWeapon->StopAnimation();} // Notify the weapons also about fail
		HealthComponent->ResetResistance();
		ReceiveInput(-1);
		break;
		
	case -2:  // Called when animation has ended in abp
		EndAnimationCheck();
		break;
		
	case -1:  // Called when an order cycle is ended
	    EndCycleCheck();
		break;
	
	case 0:  // Move and attack are handled on tick
		command = "move";
		break;
	case 1:
		if (PataponClass != "Hata") CanAttack = true; 
		break;
	case 2:
		Defend();
		break;
	case 3:
		Charge();
		break;
	case 4:
		Retreat();
		break;
	case 5:
		Hop();
		break;
	case 6:
		Dance();
		break;
	case 9:
		command = "miracle";
		break;
	default:
		command = "none";
		break;
	}
}




void APatapon_Main::Attack()  //Attack is called at a specified frame at attack animation of a class , Attack has to be made from the weapon it takes its parameters from there
{
	//Checks
	if (!WeaponClass){		LOG_OBJ(LogPatapon, this, TEXT( "has no weapon class assigned") );  return;}
	if (!Target){		LOG_OBJ(LogPatapon, this, TEXT("has no target"));   return;}     // Maybe throw random it here
	if (AttackType == "none"){		LOG_OBJ(LogPatapon, this, TEXT("has attacktype set to none"));   return;}

	
	//For ranged spawn and order to throw the projectile , For melee simply aplly damage to target

	
	//LOG_OBJ(LogPatapon, Warning, TEXT(" i am attacking to %s"), *FString(UKismetSystemLibrary::GetDisplayName(Target)));
	if (AttackType == "melee")
	{
		if (fever && charged){MainWeapon->DamageBuff = DamageBoostL2;}
		else if (fever || charged){MainWeapon->DamageBuff = DamageBoostL1;}
		MainWeapon->Melee(Target);
	}
	
	
	if (AttackType == "ranged") 
	{
		FVector ProjectileLocation = GetMesh()->GetBoneLocation("RightProp");
		FRotator ProjectileRotation = (-GetMesh()->GetBoneLocation("RightProp")+Target->GetActorLocation()).Rotation();
		AWeapon* SpawnedWeapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass,ProjectileLocation,ProjectileRotation);
		SpawnedWeapon->SetOwner(this);
		if (fever && charged){SpawnedWeapon->DamageBuff = DamageBoostL2;}
		else if (fever || charged){SpawnedWeapon->DamageBuff = DamageBoostL1;}
		
		SpawnedWeapon->Ranged(Target);
	}
	
}

void APatapon_Main::Defend()  // When defend order is given change the resistances
{
	command = "defend";
	
	if (charged && fever)
	{
		HealthComponent->AmplifyResistance(DefendBoostL3 ,ResistanceBonusL3);
	}
	
	else if (charged || fever)
	{
		HealthComponent->AmplifyResistance(DefendBoostL2 , ResistanceBonusL2);
	}
	else
	{
		HealthComponent->AmplifyResistance(DefendBoostL1 , ResistanceBonusL1);
	}
	
}

void APatapon_Main::Charge()
{
	command = "charge";
	charged = true;
}

void APatapon_Main::Retreat()
{
	command = "retreat";
	Anchor = GetActorLocation();
	
}

void APatapon_Main::RetreatEnded()  //Called from ABP
{
	command = "recover";
}

void APatapon_Main::Hop() 
{
	command = "jump";
}

void APatapon_Main::Dance() 
{
	command = "dance";
	status = "none";
	HealthComponent->ClearStatusEffect();
	HealthComponent->ClearPoisonEffect();
}

void APatapon_Main::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorCtrl, AActor* DamageCauser) // Redirected to Health component status check and response
{
	HealthComponent->ProcessDamage(Damage,DamageCauser);

	if (command == "dance") return;
	if (status != "none") return;  //Already Status
	if (PataponClass == "Hata") return; //Hatapon cannot get status effects
	
	status = HealthComponent->StatusEffect(Cast<AWeapon>(DamageCauser));
	bool isPoisoned = HealthComponent->PoisonCheck(Cast<AWeapon>(DamageCauser));
	
	//After a status is recieved 
	
	if (status != "none")
	{
		Halt(true);
		CanAttack = false;
		charged = false;
		if (LeftHandWeapon && RightHandWeapon){LeftHandWeapon->StopAnimation();RightHandWeapon->StopAnimation();}
		HealthComponent->ResetResistance();
		
		if (status == "burn"){BurnEffect->Activate();}
	}

	if (isPoisoned){PoisonEffect->Activate();}
}


int APatapon_Main::GetState()
{
	return state;
}

FString APatapon_Main::GetStatus()
{
	return status;
}

void APatapon_Main::ClearStatus()
{
	if (status == "burn"){BurnEffect->Deactivate();}
	
	if (command != "dance") // If called from dance don't change the command else try to recover from it
	{
		command = "reset";
	}
	status = "none";
	Halt();
}

void APatapon_Main::ClearPoison()
{
	PoisonEffect->Deactivate();
}

bool APatapon_Main::GetPoisioned()
{
	return HealthComponent->poisoned;
}

bool APatapon_Main::GetCanAttack()
{
	return CanAttack;
}

FString APatapon_Main::GetCommand()
{
	return command;
}

bool APatapon_Main::GetChargedUp()
{
	return charged;
}


bool APatapon_Main::GetFever()
{
	return fever;
}

void APatapon_Main::SetFever(bool SetFever)
{
	fever = SetFever;
}


AActor* APatapon_Main::GetTarget()
{
	return Target;
}


void APatapon_Main::AcquireTarget() // Get the closest simple
{
	if (BossPriority)
	{
		if(FVector::Dist(GetActorLocation(), Boss->GetActorLocation()) <= SpotRange)
		{
			Target = Boss;
		}
		return;
	}
		

	AActor* BufferedTarget=nullptr;
	for (AActor* PossibleTarget : PossibleTargets)
	{
		if (FVector::Dist(GetActorLocation(), PossibleTarget->GetActorLocation()) <= SpotRange)
		{
			if (!BufferedTarget)
			{
				BufferedTarget = PossibleTarget;
			}
			else
			{
				if(FVector::Dist(GetActorLocation(), PossibleTarget->GetActorLocation()) <= FVector::Dist(GetActorLocation(), BufferedTarget->GetActorLocation()))
				{
					BufferedTarget = PossibleTarget;
				}
			}
		}
	}
	Target = BufferedTarget;
}