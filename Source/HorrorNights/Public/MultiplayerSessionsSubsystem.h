// Sunnymoon Softwares - All rights reserved


#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
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

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    void CreateSession(int32 NumPublicConnections, FString MatchType);

    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    void CreateLANSession(int32 NumPublicConnections, FString MatchType);

    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    void FindSessions(int32 MaxSearchResults);

    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    void JoinServerByIP(FString Address);

    UFUNCTION(BlueprintPure, Category = "Multiplayer Sessions")
    bool IsLoggedIn() const { return bIsLoggedIn; }

    UPROPERTY(BlueprintAssignable, Category = "Multiplayer Sessions")
    FOnMultiplayerLoginStatusChanged OnLoginStatusChanged;

    //Delegates
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnFindSessionsComplete(bool bWasSuccessful);

protected:
    bool bIsLoggedIn = false;

    void Login();
    void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

    IOnlineSessionPtr SessionInterface;
    IOnlineIdentityPtr IdentityInterface;

    FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
    FDelegateHandle CreateSessionCompleteDelegateHandle;
    FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
    FDelegateHandle FindSessionsCompleteDelegateHandle;
    FOnLoginCompleteDelegate LoginCompleteDelegate;
    FDelegateHandle LoginCompleteDelegateHandle;

private:
    void PrintString(const FString& text);
};
