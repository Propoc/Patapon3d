#include "Weapon.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"


DEFINE_LOG_CATEGORY(LogWeapon);
#define LOG_OBJ(LogCategory,Obj, Format, ...) \
if (Obj && Obj->bDEBUG_LOG) { \
FString DisplayName = UKismetSystemLibrary::GetDisplayName(Obj); \
UE_LOG(LogCategory, Warning, TEXT("[%s] " Format), \
*DisplayName, ##__VA_ARGS__); \
}

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile"));
	
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(RootComponent);
	NiagaraComponent->bAutoActivate = false;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	WeaponMesh->OnComponentBeginOverlap.AddDynamic(this,&AWeapon::BeginOverlap);

}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (!GetOwner()) return;
	if (GetOwner()==OtherActor) return;
	float Damage = (float) FMath::RoundToInt(FMath::RandRange(MinDamage,MaxDamage));
	
	
	UGameplayStatics::ApplyDamage(OtherActor, Damage*(1+DamageBuff/100), GetOwner()->GetInstigatorController(), this, UDamageType::StaticClass());

	if (NiagaraComponent)
	{
		NiagaraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetVisibility(false);
	ProjectileMovementComp->bSimulationEnabled=false;
	GetWorldTimerManager().SetTimer(DeathTimer,this,&AWeapon::Kill,1);  // For fx 
}


void AWeapon::Melee(AActor* Target)
{
	if (Target == GetOwner()) {return;}
	float Damage = (float) FMath::RoundToInt(FMath::RandRange(MinDamage,MaxDamage));
	UGameplayStatics::ApplyDamage(Target, Damage*(1+DamageBuff/100), GetOwner()->GetInstigatorController(), this, UDamageType::StaticClass());
	DamageBuff = 0; 
}

void AWeapon::Ranged(AActor* Target) // TWO OVERLOADS WILL BE CHANGED
{
	FVector SuggestedVelocity;
	FVector StartLocation = GetActorLocation();
	
	float randomx = (Deadeye == true) ? 0 : FMath::RandRange(-Accuracy,Accuracy);
	float randomy = (Deadeye == true) ? 0 : FMath::RandRange(-Accuracy,Accuracy);
	float randomz = (Deadeye == true) ? 0 : FMath::RandRange(-Accuracy,Accuracy);


		
	FVector EndLocation = FVector(Target->GetActorLocation().X+randomx,Target->GetActorLocation().Y+randomy,Target->GetActorLocation().Z+randomz);
	bool ArcValıd = UGameplayStatics::SuggestProjectileVelocity(GetWorld(),SuggestedVelocity,StartLocation,EndLocation,ProjectileSpeed,false,0,0,ESuggestProjVelocityTraceOption::DoNotTrace);

	if(!ArcValıd)
	{
		LOG_OBJ(LogWeapon, this, TEXT(" fired by %s could not find a valid arc (Target overload)"),*FString(UKismetSystemLibrary::GetDisplayName(GetOwner())));
	}
	
	ProjectileMovementComp->Velocity=SuggestedVelocity;
	ProjectileMovementComp->bRotationFollowsVelocity=true;
	GetWorldTimerManager().SetTimer(DeathTimer,this,&AWeapon::Kill,lifetime);

	if (!NiagaraComponent){
		LOG_OBJ(LogWeapon, this, TEXT(" has no fx attached"));
		return;
	}
	NiagaraComponent->Activate(true);
	
}

void AWeapon::RangedL(FVector Location)
{
	FVector SuggestedVelocity;
	FVector StartLocation = GetActorLocation();
	
	float randomx = (Deadeye == true) ? 0 : FMath::RandRange(-Accuracy,Accuracy);
	float randomy = (Deadeye == true) ? 0 : FMath::RandRange(-Accuracy,Accuracy);
	float randomz = (Deadeye == true) ? 0 : FMath::RandRange(-Accuracy,Accuracy);


		
	FVector EndLocation = FVector(Location.X+randomx,Location.Y+randomy,Location.Z+randomz);
	bool ArcValıd = UGameplayStatics::SuggestProjectileVelocity(GetWorld(),SuggestedVelocity,StartLocation,EndLocation,ProjectileSpeed,false,0,0,ESuggestProjVelocityTraceOption::DoNotTrace);

	if(!ArcValıd)
	{
		LOG_OBJ(LogWeapon, this, TEXT(" fired by %s could not find a valid arc (Target overload)"),*FString(UKismetSystemLibrary::GetDisplayName(GetOwner())));
	}
	
	ProjectileMovementComp->Velocity=SuggestedVelocity;
	ProjectileMovementComp->bRotationFollowsVelocity=true;
	GetWorldTimerManager().SetTimer(DeathTimer,this,&AWeapon::Kill,lifetime);

	if (!NiagaraComponent){
    	LOG_OBJ(LogWeapon, this, TEXT(" has no fx attached"));
    	return;
    }
    NiagaraComponent->Activate(true);
}


void AWeapon::StartAnimation(int type)
{
	attacktype = type;
}

void AWeapon::StopAnimation()
{
	attacktype = 0;
}

int AWeapon::GetAnimationData()
{
	return  attacktype;
}


void AWeapon::Kill()
{
	Destroy();
}
