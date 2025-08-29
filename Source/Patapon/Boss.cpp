
#include "Boss.h"

#include "AIController.h"
#include "Patapon_Main.h"
#include "Health.h"

#include "Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/BlueprintTypeConversions.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogBoss);
#define LOG_OBJ(LogCategory,Obj, Format, ...) \
if (Obj && Obj->bDEBUG_LOG) { \
FString DisplayName = UKismetSystemLibrary::GetDisplayName(Obj); \
UE_LOG(LogCategory, Warning, TEXT("[%s] " Format), \
*DisplayName, ##__VA_ARGS__); \
}


ABoss::ABoss()
{
	PrimaryActorTick.bCanEverTick = true;
	
}

void ABoss::BeginPlay()
{
	Super::BeginPlay();

	SetupTargets();
	
	HealthComponent = FindComponentByClass<UHealth>();
	MovementComponent = GetCharacterMovement();
	Sphere = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("Weakpoint")));
	Weakpoint = Sphere->GetComponentLocation();
	
	OnTakeAnyDamage.AddDynamic(this, &ABoss::ReceiveDamage);
	
	AI = Cast<AAIController>(GetController());

	
	bStartIdle == true ?  status = "idle"  : status = "sleep" ;
	
	HeadButt = GetWorld()->SpawnActor<AWeapon>(HeadButtClass);
	HeadButt->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);
	HeadButt->SetOwner(this);
	HeadButt->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void ABoss::SetupTargets()
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),APatapon_Main::StaticClass(),Actors);
	for (AActor* Actor : Actors)
	{
		APatapon_Main* P = Cast<APatapon_Main>(Actor);
		Patapons.Add(P);
		if (P->PataponClass == "Hata")
		{
			Hatapon = P;
		}
	}

}


void ABoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!Hatapon)
	{
		SetupTargets();
		return;
	}
	
	DistanceToHatapon = FVector::Dist(GetActorLocation(), Hatapon->GetActorLocation());
	Weakpoint = Sphere->GetComponentLocation();  //This needs to be updated so patapons can reach it

	if (status == "sleep" && DistanceToHatapon <= detectRange) // If sleeping check for wake up , reseted after animation at abp
	{
		status = "wakeup";
	}


	
	if (bBreathing)  
	{
		if (BreatheGap == 0)
        {
			Breathe = GetWorld()->SpawnActor<AWeapon>(BreatheClass,GetMesh()->GetSocketTransform("FireSocket"));
			Breathe->SetOwner(this);
			FVector SocketLoc;
			FRotator SocketRot;
			GetMesh()->GetSocketWorldLocationAndRotation("FireSocket",SocketLoc,SocketRot);
			FVector ForwardVector = SocketRot.Vector();
			FVector TargetLocation = SocketLoc + (ForwardVector * 3000);
			TargetLocation.Z = Hatapon->GetActorLocation().Z-300;
			Breathe->RangedL( TargetLocation);
			BreatheGap = 2;
        }
		else
		{
			BreatheGap -=1;
		}

	}

}


// 0. peak son ordersa 1in başında order başlıyo 4ün sonunda bitiyor. Sonraki emir 4 peak sürdüğünden yani 2 order arası 150 - 2*grace
// beat window daha da kısa boş sekmeleri atınca geriye kalan.
// YANİ ilk counterda basılınca 120-grace kadar tepki süresi var 120 + grace de diğer commandin 1.peakine kadar olan süre burdan. Command 1 grace daha erken giricek peakten
// YANİİİİ 120-grace tepki süresi + 120 commandin minumum devreye girişi + animasyonun vurduğu an ve toparlanma frameleri kalıyor.
// 2. framede başlatıyorum. 90-grace + 120 +  grace + 30 (1.framede vursun mesela)  200 - 310 arasında olmaı

void ABoss::CycleInfo(int CommandCounter)  // Check every 30 frames
{
	if (!Hatapon) return;
	
	if (bDEBUG_RANGE){
		DrawDebugSphere(GetWorld(), GetActorLocation(), detectRange, 16, FColor::Red, false, 1, 0, 3);
		DrawDebugSphere(GetWorld(), GetActorLocation(), attackRange, 16, FColor::Black, false, 1, 0, 3);
	}
	
	if (status == "sleep" || status == "wakeup")
	{
		return;
	}

	if (status == "attack")
	{
		LOG_OBJ(LogBoss, this, TEXT("Already attacking - early return"));
		return;
	}

	if (bAttackReady) CycleAttackTimingSafe +=1;
	
	
	if (!bAttackReady)    // When does the attack animation ends is changing so to recalibrate 
	{
		CycleWithoutAttackSafe +=1;

		if (CommandCounter==1) //Assuming Combo does not break
		{
			CycleWithoutAttack +=1;
			CycleWithoutAttackSafe=0;

			
			if (CycleWithoutAttack == 1) {AttackWeight = 0.1;}       // 1        0.1
			else if (CycleWithoutAttack == 2) {AttackWeight = 0.4;}  // 1'*2     0.9*0.4 = 0.36
			else if (CycleWithoutAttack == 3) {AttackWeight = 0.8;}  // 1'*2'*3  0.9*0.6*0.8 = 0.432
			else {AttackWeight = 1;}                                 // 1 - E    0.108

			if (UKismetMathLibrary::RandomBoolWithWeight(AttackWeight))
			{
				bAttackReady = true;
			}
			
		}
		else if (CycleWithoutAttackSafe >= 10)
		{
			CycleWithoutAttack +=1;
			CycleWithoutAttackSafe=0;

			if (CycleWithoutAttack == 1) {AttackWeight = 0.1;}       // 1        0.1
			else if (CycleWithoutAttack == 2) {AttackWeight = 0.4;}  // 1'*2     0.9*0.4 = 0.36
			else if (CycleWithoutAttack == 3) {AttackWeight = 0.8;}  // 1'*2'*3  0.9*0.6*0.8 = 0.432
			else {AttackWeight = 1;}                                 // 1 - E    0.108

			if (UKismetMathLibrary::RandomBoolWithWeight(AttackWeight))
			{
				bAttackReady = true;
			}
			
		}
		
		
	}
	

	if (status == "jump")
	{
		LOG_OBJ(LogBoss, this, TEXT("Already jumping - early return"));
		return;
	}

	
	 
	CycleCooldown -= 1;
	if (CycleCooldown < 0)
	{
		CycleCooldown = 0;

	}
	else
	{
		LOG_OBJ(LogBoss, this, TEXT("Doing an action with a cooldown - early return"));
		return;
	}

	

	
	if (DistanceToHatapon >= detectRange)    // If it is too far away keep walking;
	{
		LOG_OBJ(LogBoss, this, TEXT(" MOVE - It is too far to detect anything"));
		Move(Hatapon);
		CycleCooldown = 1;
	}

	
	else if (DistanceToHatapon <= detectRange && FVector::Dist(GetClosestPatapon()->GetActorLocation(),GetActorLocation()) <= attackRange) // Hatapon in range also something in my attack range
	{
		if (bAttackReady)  // If ready to attack wait for the reactable time
		{
			if (CommandCounter == 2 || CycleAttackTimingSafe == 3)
			{
				MovementComponent->StopActiveMovement();
				status = "attack";
			
				if(UKismetMathLibrary::RandomBoolWithWeight(0.5))
				{
					attacktype = "headbutt";
				}
				else if(UKismetMathLibrary::RandomBoolWithWeight(1))
				{
					attacktype = "breathe";
				}
				else
				{
					attacktype = "devour";
				}
			
			 
				if (DEBUG_ATTACK != "none"){attacktype = DEBUG_ATTACK;} //Little Debug
			
				CycleWithoutAttack = 0;
				CycleWithoutAttackSafe = 0;
				CycleAttackTimingSafe = 0;
				bAttackReady = false;
		
				LOG_OBJ(LogBoss, this, TEXT(" I am attacking - %s ") , *FString(attacktype));

			}
			else
			{
				CycleAttackTimingSafe +=1;
			}
			
		
		}

		
		
		else
		{
			MoveWeight = (DistanceToHatapon-attackRange)/detectRange / 4 ;  // This gives a normalized value but dont always run if it is max range so a divider

			if (CycleWithoutAttack == 0 && UKismetMathLibrary::RandomBoolWithWeight(0.5))  //Run back maybe if just attacked
			{
				LOG_OBJ(LogBoss, this, TEXT(" I will back away a chance of %f "),MoveWeight);
				FVector Normal = Hatapon->GetActorLocation()-GetActorLocation();
				Normal.Normalize();
				FVector Step = GetActorLocation() - Normal * 5000;
				Back(Step);
				CycleCooldown = 1;
			}
			
			
			else if(UKismetMathLibrary::RandomBoolWithWeight(MoveWeight))  //First lets move if it is more convinent
			{
				LOG_OBJ(LogBoss, this, TEXT(" I will move with a chance of %f "),MoveWeight);
				Move(Hatapon);
				CycleCooldown = 1;
			}

			else
			{
				IdleWeight = 0.8;
				
				if(UKismetMathLibrary::RandomBoolWithWeight(IdleWeight))  //Do nothing thats an option
				{
					LOG_OBJ(LogBoss, this, TEXT(" I will stand still with a chance of %f "),(1-MoveWeight)*IdleWeight);
					MovementComponent->StopActiveMovement();
					status = "idle";
					CycleCooldown = 2;
				}
				else  // Jump is 1-Idle 
				{
					LOG_OBJ(LogBoss, this, TEXT(" I will jump with a chance of %f "),(1-MoveWeight)*IdleWeight);
					MovementComponent->StopActiveMovement();
					status = "jump";
				}
			}
		}
	}

	else  //Hatapon in range but not a target in range
	{
			
		if (bAttackReady)
		{
			LOG_OBJ(LogBoss, this, TEXT("My attack ready closing gap"))
			Move(Hatapon);
			CycleCooldown = 1;
		}
		
		else
		{
			MoveWeight = (DistanceToHatapon-attackRange)/detectRange ; // Farther the range more incentivized to move
			
			if(UKismetMathLibrary::RandomBoolWithWeight(MoveWeight))  //First lets move if it is more convinent
			{
				LOG_OBJ(LogBoss, this, TEXT(" I will close in with a chance of %f "),MoveWeight);
				Move(Hatapon);
				CycleCooldown = 1;
			}

			else
			{
				IdleWeight = 0.8;
				
				if(UKismetMathLibrary::RandomBoolWithWeight(IdleWeight)) 
				{
					LOG_OBJ(LogBoss, this, TEXT("Nothing in attack range - I will stand still with a chance of %f "),(1-MoveWeight)*IdleWeight);
					MovementComponent->StopActiveMovement();
					status = "idle";
					CycleCooldown = 2;
				}
				else  // Jump is 1-Idle 
				{
					LOG_OBJ(LogBoss, this, TEXT(" Nothing in attack range - I will jump with a chance of %f"),(1-MoveWeight)*(1-IdleWeight));
					MovementComponent->StopActiveMovement();
					status = "jump";
				}
			}
		}
		
	}
	
}

void ABoss::Move(APatapon_Main* Target)
{
	status = "move";
	FVector To = Target->GetActorLocation();
	AI->MoveToLocation(To,0.1);
}

void ABoss::Back(FVector To)
{
	status = "move";
	
	FVector ToTarget = Hatapon->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0;
	
	if (ToTarget.IsNearlyZero()) return;
	FRotator DesiredRotation = ToTarget.Rotation();

	SetActorRotation(DesiredRotation);
	
	AI->MoveToLocation(To,0.1);
}


void ABoss::ApplyDamage(AActor* whom)
{
	HeadButt->Melee(whom);
}

void ABoss::SetBreathe(bool bSet)
{
	bBreathing = bSet;
}

void ABoss::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,AController* InstigatorCtrl, AActor* DamageCauser)
{
	HealthComponent->ProcessDamage(Damage,DamageCauser);
}

APatapon_Main* ABoss::GetClosestPatapon() // Get the closest simple
{
	AActor* BufferedTarget=nullptr;
	for (AActor* PossibleTarget : Patapons)
	{
		if (FVector::Dist(GetActorLocation(), PossibleTarget->GetActorLocation()) <= 50000)
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
	return Cast<APatapon_Main>(BufferedTarget);
}
