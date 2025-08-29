// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Pawn_Main.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogPawn, Log, All);

UENUM(BlueprintType)
enum class EInputFailType : uint8
{
	GivenBeatAlready,
	CommandInProgress,
	OutOfCycle,
	NotAValidCommand,
	MissedABeat,
	DroppedCombo
};



UCLASS()
class PATAPON_API APawn_Main : public APawn
{
	GENERATED_BODY()

public:
	APawn_Main();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere , Category = "Debug")
	bool bDEBUG_LOG = false;
	// Public for UI
		
	UPROPERTY(VisibleAnywhere , BlueprintReadOnly)
	bool bActive = false;
	UPROPERTY(VisibleAnywhere  , BlueprintReadOnly)
	int frame = 0;
	UPROPERTY(EditAnywhere  , BlueprintReadOnly)
	int cycle_frame = 30;
	UPROPERTY(EditAnywhere  , BlueprintReadOnly)
	int grace_frames = 5;


	UPROPERTY(EditAnywhere  , BlueprintReadWrite)
	int InputInfoUI = 0;

	UFUNCTION(BlueprintPure)
	int GetCombo();
	UFUNCTION(BlueprintPure)
	bool GetCycle();
	UFUNCTION(BlueprintPure)
	bool GetCommandInProgress();

	UFUNCTION(BlueprintImplementableEvent)
	void InputInfo(int button , int precision);

	UFUNCTION(BlueprintImplementableEvent)
	void BeatInfo(bool ReadyToSwitch , int CommandActive ,int combo , bool FeverActive);

	UFUNCTION(BlueprintImplementableEvent)
	void StartBlueprint();
	FTimerHandle StartTimer;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<class APatapon_Main*> Patapons;
	UPROPERTY(BlueprintReadWrite)
	APatapon_Main* Hatapon;

	class ABoss* Boss;
	
private:
	
	
	//Components
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* BaseMesh;
	
	
	//Initilaizing the startup
	void StartWorldTimer();
	void SetupPatapons();
	void Arrange(TArray<class APatapon_Main*> Patapons , FVector point , FVector refpoint ,float arrangementid=0);


	UPROPERTY(EditAnywhere , Category = "Setup")
	float RankDifference = 300;
	UPROPERTY(EditAnywhere , Category = "Setup")
	float RankLenght = 50;
	UPROPERTY(EditAnywhere , Category = "Setup")
	float RankWidth  = 100;
	
	
	//Taken from blueprint inputs
	
	
	UFUNCTION(BlueprintCallable)
	void TakeInput(int Input);
	UFUNCTION(BlueprintCallable)
	void CommandShortcut(int Input);
	

	//Broadcasts to Pawns
	
	void InputFailed(int Input , EInputFailType type);
	void BroadcastOrder(int order);
	void BroadcastInput(int command);
	void BroadcastFever(bool fever);

	//Frame controlls

	void ProcessFrame();

	
	bool bCycleReady = false;
	bool bBeatRecieved = false;
	bool bCommandPending = false;
	bool bCommandInProgress = false;
	int ActiveCommand = -1 ;

	
	bool MiracleInEffect = false;
	
	int InputCounter = 0;
	int CommandCounter = 0;
	int Combo = 0;
	int FeverCombo = 10;
	bool Fever = false;

	int ComboPerfection;
	
	//Command list
	
	int movecommand[4] = {1,1,1,2};
	int attackcommand[4] = {2,2,1,2};
	int defendcommand[4] = {4,4,1,2};
	int chargecommand[4] = {2,2,4,4};
	int retreatcommand[4] = {2,1,2,1};
	int jumpcommand[4] = {3,3,4,4};
	int dancecommand[4] = {1,2,3,4};
	int commands[9][4] = {{1,1,1,2},{2,2,1,2},{4,4,1,2},{2,2,4,4},{2,1,2,1},{3,3,4,4},{1,2,3,4},{3,33,33,0},{3,33,3,0}};

	int commandbuffer[4] = {-1,-1,-1,-1};
	void CleanArrays();


	
};

