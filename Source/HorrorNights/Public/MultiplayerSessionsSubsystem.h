// Sunnymoon Softwares - All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"

UCLASS()
class HORRORNIGHTS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UMultiplayerSessionsSubsystem();

	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

private:
	IOnlineSubsystem* OnlineSubsys;
	IOnlineSessionPtr SessionInterface;

	void OnCreateSessionComplete(FName sessionName, bool wasSuccessful);
	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type joinResult);
	void PrintString(const FString& text);

	UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
	void CreateServer(FString server);

	UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
	void JoinServer(FString server);
};
