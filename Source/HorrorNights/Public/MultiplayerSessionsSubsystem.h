// Sunnymoon Softwares - All rights reserved


#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "FindSessionsCallbackProxy.h"
#include "MultiplayerSessionsSubsystem.generated.h"

// A UI pode dar Bind neste evento pra saber quando o login no EOS terminou
// (com sucesso ou não) e só então liberar os botões de multiplayer.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMultiplayerLoginStatusChanged, bool, bWasSuccessful);

UCLASS()
class HORRORNIGHTS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
    FOnlineSessionSearchResult HostFoundSession;

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    void CreateSession(int32 NumPublicConnections, FString MatchType);

    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    void CreateLANSession(int32 NumPublicConnections, FString MatchType);

    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    void FindSessions(int32 MaxSearchResults);

	UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    void JoinSession();

    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    void JoinServerByIP(FString Address);

    UFUNCTION(BlueprintPure, Category = "Multiplayer Sessions")
    bool IsLoggedIn() const { return bIsLoggedIn; }

    UPROPERTY(BlueprintAssignable, Category = "Multiplayer Sessions")
    FOnMultiplayerLoginStatusChanged OnLoginStatusChanged;

    //Delegates
    void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

protected:
    bool bIsLoggedIn = false;
	bool bIsLanSession = false;

    void Login();

    IOnlineSessionPtr SessionInterface;
    IOnlineIdentityPtr IdentityInterface;

    FOnLoginCompleteDelegate LoginCompleteDelegate;
    FDelegateHandle LoginCompleteDelegateHandle;
    FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
    FDelegateHandle CreateSessionCompleteDelegateHandle;
    FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
    FDelegateHandle FindSessionsCompleteDelegateHandle;
    FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
    FDelegateHandle JoinSessionCompleteDelegateHandle;

private:
    void PrintString(const FString& text);
};
