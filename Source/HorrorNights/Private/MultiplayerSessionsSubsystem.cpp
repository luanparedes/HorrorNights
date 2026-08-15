// Sunnymoon Softwares - All rights reserved


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
{
	PrintString("Online subsystem constructor called!");
}

void UMultiplayerSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	OnlineSubsys = IOnlineSubsystem::Get();

	if (OnlineSubsys)
	{
		FString SubsystemName = OnlineSubsys->GetSubsystemName().ToString();

		SessionInterface = OnlineSubsys->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnJoinSessionComplete);
			PrintString(FString::Printf(TEXT("Found online subsystem: %s"), *SubsystemName));
		}
		else
		{
			PrintString("Session interface is NOT valid!");
		}
	}
	else
	{
		PrintString("No online subsystem found!");
	}
}

void UMultiplayerSessionsSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Warning, TEXT("Online subsystem deinitialized!"));
}

void UMultiplayerSessionsSubsystem::CreateServer(FString server)
{
	bool isLan = false;

	if (server.IsEmpty())
	{
		PrintString("Server name is empty!");
		return;
	}

	if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		isLan = true;
	}

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.NumPublicConnections = 4;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bIsLANMatch = isLan;

	SessionInterface->CreateSession(0, FName(*server), SessionSettings);
}

void UMultiplayerSessionsSubsystem::JoinServer(FString server)
{
	PrintString("Joining Server!");
	//SessionInterface->JoinSession(0, FName(*server), SessionSearchResult);
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName sessionName, bool wasSuccessful)
{
	if (wasSuccessful)
	{
		PrintString("Session created successfully!");
		GetWorld()->ServerTravel("/Game/Maps/Level_1-1?listen");
	}
	else
	{
		PrintString("Failed to create session!");
	}
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type joinResult)
{
	if (joinResult != EOnJoinSessionCompleteResult::Success)
	{
		PrintString("Failed to join session!");
		return;
	}
}

void UMultiplayerSessionsSubsystem::PrintString(const FString& text)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, text);
	}
}