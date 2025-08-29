#include "Pawn_Main.h"

#include "Boss.h"
#include "Kismet/GameplayStatics.h"
#include "Patapon_Main.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogPawn);
#define LOG_OBJ(LogCategory,Obj, Format, ...) \
if (Obj && Obj->bDEBUG_LOG) { \
FString DisplayName = UKismetSystemLibrary::GetDisplayName(Obj); \
UE_LOG(LogCategory, Warning, TEXT("[%s] " Format), \
*DisplayName, ##__VA_ARGS__); \
}

APawn_Main::APawn_Main()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bHighPriority = true;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));
	RootComponent = BaseMesh;
}

void APawn_Main::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(StartTimer,this,&APawn_Main::StartWorldTimer,0.05);

}

void APawn_Main::StartWorldTimer()  // In Packaged build if called in begin play fatal error
{
	SetupPatapons();
	bActive = true;
	StartBlueprint();
}

void APawn_Main::SetupPatapons()  //Levelde Hataponun konumuna göre yerleştirme sekansı + Boss
{

	Boss = Cast<ABoss>(UGameplayStatics::GetActorOfClass(GetWorld(), ABoss::StaticClass()));
	
	
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),APatapon_Main::StaticClass(),Actors);
	for (AActor* Actor : Actors)
	{
		APatapon_Main* P = Cast<APatapon_Main>(Actor);
		if (P->Team==1)
		{
			Patapons.Add(P);
		}  
	}
		
	FVector RankFlag;
	FVector RankRotation;
	TArray<APatapon_Main*> RankFirsts;
	TArray<APatapon_Main*> RankSeconds;
	TArray<APatapon_Main*> RankThirds;

	for (APatapon_Main* Patapon : Patapons)  
	{
		if (Patapon->PataponClass == "Hata")
		{
			Hatapon = Patapon;
			Patapon->RelativeLocation = Patapon->GetActorLocation();
			RankFlag = Patapon->GetActorLocation();
			RankRotation = Patapon->GetActorForwardVector();
		}
		else if (Patapon->PataponClass == "Tate")
		{
			RankFirsts.Add(Patapon);
		}
		else if (Patapon->PataponClass == "Yari")
		{
			RankSeconds.Add(Patapon);
		}
		else if (Patapon->PataponClass == "Yumi")
		{
			RankThirds.Add(Patapon);
		}
	}
	for (APatapon_Main* Patapon : Patapons)   // Hata needs to be defined first thats why here
	{
		Patapon->Hatapon = Hatapon;
	}
	
	
	
	FVector RankFirst = RankFlag + RankDifference * RankRotation * 2;
	FVector RankSecond = RankFlag + RankDifference * RankRotation * 1;
	FVector RankThird = RankFlag - RankDifference * RankRotation * 1;


	//Z için değiştirmek lazım
	Arrange(RankFirsts,RankFirst , RankSecond);
	Arrange(RankSeconds, RankSecond , RankFlag);
	Arrange(RankThirds, RankThird , RankThird-(RankFlag-RankThird));
	
	for (APatapon_Main* Patapon : Patapons)
	{
		Patapon->PrimaryActorTick.bCanEverTick = true;
		Patapon->InitialSetup();
	}
}

void APawn_Main::Arrange(TArray<APatapon_Main*> PataponsToArrange, FVector point, FVector refpoint, float arrangementid)
{
	if (arrangementid == 0) //chevron
	{
		int Size = PataponsToArrange.Num();
		if (Size % 2 == 0) // Even
		{
			float i = 1;
			FVector rotatedpoint;
			for (APatapon_Main* P : PataponsToArrange)  
			{
				
				float beta = UKismetMathLibrary::Atan((RankWidth*(abs(i)-0.5)) / (RankDifference-RankLenght*(abs(i)-1)));
				float phi = UKismetMathLibrary::Atan((point.Y-refpoint.Y)/(point.X-refpoint.X));

				float newphi = phi + beta*abs(i)/i ;
				float x = sqrt(pow(RankDifference-RankLenght*(abs(i)-1),2)+pow(RankWidth*(abs(i)-0.5),2))*UKismetMathLibrary::Cos(newphi);
				float y = sqrt(pow(RankDifference-RankLenght*(abs(i)-1),2)+pow(RankWidth*(abs(i)-0.5),2))*UKismetMathLibrary::Sin(newphi);

				
				rotatedpoint = FVector(refpoint.X + x ,refpoint.Y + y , point.Z);
				P->SetActorLocation(rotatedpoint);
				FVector facingvector = FVector(point.X-refpoint.X,point.Y-refpoint.Y,refpoint.Z);
				facingvector.Normalize();
				P->SetActorRotation(facingvector.Rotation());
				
				P->RelativeLocation = rotatedpoint - Hatapon->GetActorLocation();
				
				if (i>0)
				{
					i=-i;
				}
				else if (i<0)
				{
					i=-i;
					i+=1;
				}
			}
		}
		else   //Odd
		{
			float i = 0;
			FVector rotatedpoint;
			for (APatapon_Main* P : PataponsToArrange)  
			{
				if (i == 0)
				{
					rotatedpoint = point;
				}
				else
				{
					float beta = UKismetMathLibrary::Atan((RankWidth*i) / (RankDifference-RankLenght*abs(i)));
					float phi = UKismetMathLibrary::Atan((point.Y-refpoint.Y)/(point.X-refpoint.X));

					float newphi = phi + beta ;
					float x = sqrt(pow(RankDifference-RankLenght*abs(i),2)+pow(RankWidth*i,2))*UKismetMathLibrary::Cos(newphi);
					float y = sqrt(pow(RankDifference-RankLenght*abs(i),2)+pow(RankWidth*i,2))*UKismetMathLibrary::Sin(newphi);


					rotatedpoint = FVector(refpoint.X + x ,refpoint.Y + y , point.Z);
					
				}
				
				P->SetActorLocation(rotatedpoint);
				FVector facingvector = FVector(point.X-refpoint.X,point.Y-refpoint.Y,refpoint.Z);
				facingvector.Normalize();
				P->SetActorRotation(facingvector.Rotation());
				
				P->RelativeLocation = rotatedpoint - Hatapon->GetActorLocation();
				
				if (i == 0)
				{
					i=1;
				}
				else if (i>0)
				{
					i=-i;
				}
				else if (i<0)
				{
					i=-i;
					i+=1;
				}
			}
		}
	}
}


void APawn_Main::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bActive)
	{
		ProcessFrame();
	}	
}


// 60 FPS Oyun 120bpm 1 sekans 30 frame sürüyor so mod(30) da  30 peak (30-grace) rising edge (30+grace) falling edge order doğruysa vermek için last frame checki bekliyor, anında da verilebilir ?*
void APawn_Main::ProcessFrame()  
{
	frame+=1;
	if (frame>cycle_frame){frame=1;}

	if (frame==cycle_frame)  //Peak
	{
		bCycleReady=true;
		if (bCommandInProgress)
		{
			CommandCounter+=1;
		}
		
		BeatInfo(bCommandInProgress && CommandCounter == 1 , ActiveCommand, Combo , Fever); // Müzik 120bpm sequenceların burda tetiklenmesi lazım combo müziği blendlesin mi bi sonrakine yoksa beklesin mi ?*
		Boss->CycleInfo(CommandCounter);
	}

	/// I WILL CHECK THIS !!!!!!!!!!!!!!!!!!!!!!!!!!!!! DEPENDS ON WHICH CODE RUNS FIRST (MAYBE BEACUSE INPUTS ARE IN BP WILL CHANGE)   
	else if(frame==grace_frames+1) // Checks after +1 (Window Closed) 
	{
		bCycleReady=false;  
		
		if (bCommandInProgress)   //Order Cyclelarında 
		{
			if (CommandCounter == 4)
			{
				CommandCounter = 0;
				bCommandInProgress = false;
				ActiveCommand = -1;
				ComboPerfection = 0;
				BroadcastOrder(-1);  //Notifyla Command Bitti
			}
		}
		else
		{
			if (!bBeatRecieved)  //Not recieved a order in sequence
			{
				if (Combo == 0 || Combo == 1)  // If combo hasnt started dont fail the input 
				{
					InputCounter = 0;
					ComboPerfection = 0;
					Combo = 0;
					CleanArrays();
					BroadcastOrder(-9);
					LOG_OBJ(LogPawn, this, TEXT("Beat Missed but not triggering an input fail because its Combo : %d"),Combo);
				}

				else
				{
					if (InputCounter == 1)  // Error feedbacki ilki kaçtı ya da arada kaçırdı
					{
						InputFailed(0,EInputFailType::DroppedCombo);
					}
					else 
					{
						InputFailed(0,EInputFailType::MissedABeat);
					}

				}
			}
		}
	}
	
	else if (frame == cycle_frame-grace_frames)  // Opened - First Check
	{
		bCycleReady=true;
		
		if (bCommandPending)  // Order starts here
		{
			LOG_OBJ(LogPawn, this, TEXT("Order started at frame = %d") , frame);
			
			bCommandInProgress = true;
			InputCounter = 0;

			if (ComboPerfection <= 5)   //Combo perfection check
			{
				Combo += 4;
			}
			else if (ComboPerfection <= 10)
			{
				Combo += 3;
			}
			else if (ComboPerfection <= 15)
			{
				Combo += 2;
			}
			else
			{
				Combo += 1;
			}

			LOG_OBJ(LogPawn, this, TEXT("Got Combo Score of %d"),ComboPerfection);
			
			if (!Fever)
			{
				if (Combo >= FeverCombo)
				{
					Combo = FeverCombo;
					Fever = true;
					BroadcastFever(true);
				}
			}
			else
			{
				Combo = FeverCombo;
			}
						
			BroadcastOrder(ActiveCommand);
			
			CleanArrays();
			bCommandPending = false;
		}
	}

	
	else if(frame>=cycle_frame-grace_frames)  //Rising Edge 
	{
		bCycleReady=true;
	}
	
	else if(frame<=grace_frames)  //Falling Edge
	{
		bCycleReady=true;
	}
	
	else  //Closed
	{
		bCycleReady=false;
		bBeatRecieved = false;
		InputInfoUI = 0;
	}		
	
}



void APawn_Main::TakeInput(int Input)  // Input Tuşlarını Cpp ye taşımam lazım şu anlık Blueprintten tetikleniyo, Input broadcasts immediately
{
	// Tamemen kolpa çalışıyor saçma sapan arrayler var düzeltmek lazım
	
	if (!bCycleReady) {InputFailed(Input,EInputFailType::OutOfCycle);  return;};
	if (bCommandInProgress) {InputFailed(Input,EInputFailType::CommandInProgress);  return;}
	if (bBeatRecieved && Input == 3 )
	{
		InputCounter-=1;  //trick
		commandbuffer[InputCounter-1] += 30;
		goto skip;
	}
	if (bBeatRecieved) 	{InputFailed(Input,EInputFailType::GivenBeatAlready);  return;};
	
	bBeatRecieved = true;
	InputCounter += 1;
	commandbuffer[InputCounter-1] = Input;

	skip:
	for  (int i=0; i<9 ; i++)
	{
		for (int k=0; k<InputCounter ; k++)
		{
			if (commandbuffer[k] != commands[i][k])
			{
				break;
			}

			if (k==3)
			{
				bCommandPending = true;
				ActiveCommand = i;
			}
			if (k == InputCounter-1)
			{
				goto jump;
			}

		}
	}
	
	InputFailed(Input,EInputFailType::NotAValidCommand);
	return;
	
	jump:

	int difference = frame <= 15 ? frame : cycle_frame - frame;

	if (difference == 0)
	{
		LOG_OBJ(LogPawn, this, TEXT("This is frame = %d , Perfect beat"), frame);
	}
	else
	{
		LOG_OBJ(LogPawn, this, TEXT("This is frame = %d , Beat off by = %d"), frame ,difference);
	}

	
	if (InputCounter == 4)
	{
		ComboPerfection += difference*2;
	}
	else
	{
		ComboPerfection += difference;
	}
	
	InputInfo(Input,difference);
	
	if (Input==1)
	{
		BroadcastInput(1);
	}
	if (Input==2)
	{
		BroadcastInput(2);
	}
	if (Input==3)
	{
		BroadcastInput(3);
	}
	if (Input==4)
	{
		BroadcastInput(4);
	}

	InputInfoUI = Input;
}



void APawn_Main::InputFailed(int Input, EInputFailType type)
{
	LOG_OBJ(LogPawn, this, TEXT("Failed at frame (%d), Reason = %s"), frame, *UEnum::GetValueAsString(type));

	InputInfo(Input,-1);
	
	InputCounter = 0;
	CommandCounter = 0;
	Combo = 0;
	ComboPerfection = 0;
	
	ActiveCommand = -1;
	bCommandInProgress = false;
	Fever = false;
	bCommandPending = false;
	
	CleanArrays();
	
	BroadcastOrder(-3);
	BroadcastFever(false);

	InputInfoUI = -1;
}

//  Interfacele düzeltilebilir ama zaten hepsi cachete hangisi hızlı ?*

void APawn_Main::BroadcastOrder(int order)
{
	LOG_OBJ(LogPawn, this, TEXT(" This is frame = (%d) , Order Given is = %d"), frame , order);
	
	for (APatapon_Main* Patapon : Patapons)
	{
		Patapon->ReceiveCommand(order);
	}
	

}

void APawn_Main::BroadcastInput(int command)
{
	for (APatapon_Main* Patapon : Patapons)
	{
		Patapon->ReceiveInput(command);
	}
}

void APawn_Main::BroadcastFever(bool fever)
{
	for (APatapon_Main* Patapon : Patapons)
	{
		Patapon->SetFever(fever);
	}
}



int APawn_Main::GetCombo()
{
	return Combo;
}

bool APawn_Main::GetCycle()
{
	return bCycleReady;
}

bool APawn_Main::GetCommandInProgress()
{
	return bCommandInProgress;
}



void APawn_Main::CleanArrays()
{
	for (int i=0; i<4 ; i++)
	{
		commandbuffer[i] = -1;
	}
}

void APawn_Main::CommandShortcut(int Input) // Ugly debug
{

	if (bCommandInProgress)
	{
		LOG_OBJ(LogPawn , this, TEXT("ORDER IS IN EFFECT WAIT"));
		return;
	}
	bBeatRecieved = true;
	bCommandPending = true;
	ComboPerfection = 99;
	if (Input == 9) {ActiveCommand = 0; return;}
	ActiveCommand = Input;
}
