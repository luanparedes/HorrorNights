// Sunnymoon Softwares - All rights reserved


#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "FindSessionsCallbackProxy.h"
#include "Containers/Ticker.h"
#include "MultiplayerSessionsSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FHNSessionInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Multiplayer Sessions")
    int32 ResultIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category = "Multiplayer Sessions")
    FString ServerName;

    UPROPERTY(BlueprintReadOnly, Category = "Multiplayer Sessions")
    FString HostName;

    UPROPERTY(BlueprintReadOnly, Category = "Multiplayer Sessions")
    int32 CurrentPlayers = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Multiplayer Sessions")
    int32 MaxPlayers = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Multiplayer Sessions")
    FString MatchType;

    // Level atual da sala, no padrão Level_<Temporada>-<Fase>. 0 = desconhecido.
    UPROPERTY(BlueprintReadOnly, Category = "Multiplayer Sessions")
    int32 Season = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Multiplayer Sessions")
    int32 Stage = 0;

    // Pronto pra exibir: "1-1". Vazio se o level for desconhecido.
    UPROPERTY(BlueprintReadOnly, Category = "Multiplayer Sessions")
    FString LevelName;

    // Sala sem vagas. Só aparece na busca LAN (no online salas cheias são escondidas).
    // Use para desabilitar o botão "Entrar" e mostrar "Cheia".
    UPROPERTY(BlueprintReadOnly, Category = "Multiplayer Sessions")
    bool bIsFull = false;

    // true = sala da rede local (LAN), false = sala online (EOS).
    UPROPERTY(BlueprintReadOnly, Category = "Multiplayer Sessions")
    bool bIsLan = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMultiplayerLoginStatusChanged, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHNSessionsFound, const TArray<FHNSessionInfo>&, Sessions, bool, bWasSuccessful);
// Disparado quando não dá pra entrar numa sala. Reason já vem pronto pra mostrar na tela.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHNJoinFailed, const FText&, Reason);

UCLASS()
class HORRORNIGHTS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    static constexpr int32 MaxServerNameLength = 24;
    static constexpr int32 MaxSessionsToDisplay = 20;
    static constexpr double MinSecondsBetweenSearches = 5.0;

    TSharedPtr<FOnlineSessionSearch> LastSessionSearch;      // última busca online (EOS)
    TSharedPtr<FOnlineSessionSearch> LastLANSessionSearch;   // última busca LAN (NULL)
    FOnlineSessionSearchResult HostFoundSession;

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Cria a sala e abre /Game/Maps/Level_<Season>-<Stage>.
    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    void CreateSession(int32 NumPublicConnections, FString MatchType, FString ServerName = TEXT(""), int32 Season = 1, int32 Stage = 1);

    // Cria uma sala na rede local. ServerName vazio = "<Nome do host>'s Room".
    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    void CreateLANSession(int32 NumPublicConnections, FString MatchType, FString ServerName = TEXT(""), int32 Season = 1, int32 Stage = 1);

    // Procura salas na rede local (mesmo Wi-Fi/roteador). Não precisa de login.
    // O resultado chega no OnLANSessionsFound, incluindo salas cheias (bIsFull).
    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    bool FindLANSessions(int32 MaxSearchResults = 50);

    // Jogo rápido na rede local: entra na primeira sala LAN com vaga ou cria uma.
    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    bool QuickJoinLAN(bool bCreateIfNoneFound = true, int32 NumPublicConnections = 4, int32 Season = 1, int32 Stage = 1);

    // Atualiza o level anunciado na sala. Normalmente não precisa chamar: quando o host
    // carrega um mapa Level_X-Y, a sala é atualizada sozinha.
    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    bool UpdateSessionLevel(int32 Season, int32 Stage);

    // Lê temporada e fase de um nome de mapa no padrão "Level_1-2" (aceita caminho completo e prefixo de PIE).
    UFUNCTION(BlueprintPure, Category = "Multiplayer Sessions")
    static bool ParseLevelName(const FString& MapName, int32& Season, int32& Stage);

    // Temporada e fase do mapa carregado agora. Retorna false se o mapa não segue o padrão (ex: menu).
    UFUNCTION(BlueprintPure, Category = "Multiplayer Sessions")
    bool GetCurrentLevel(int32& Season, int32& Stage) const;

    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    bool FindSessions(int32 MaxSearchResults = 100);

    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    bool QuickJoin(int32 MaxSearchResults = 100, bool bCreateIfNoneFound = true, int32 NumPublicConnections = 4, int32 Season = 1, int32 Stage = 1);

    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    bool JoinSessionByIndex(int32 ResultIndex, bool bLan = false);

    // Entra na sala da lista (online ou LAN). Use no botão "Entrar" de cada linha.
    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    bool JoinRoom(const FHNSessionInfo& Room);

    UFUNCTION(BlueprintPure, Category = "Multiplayer Sessions")
    bool IsSearchingLANSessions() const { return bIsSearchingLAN; }

    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    void JoinSession();

    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    void JoinServerByIP(FString Address);

    UFUNCTION(BlueprintPure, Category = "Multiplayer Sessions")
    bool IsLoggedIn() const { return bIsLoggedIn; }

    // Abre o login da Epic (se ainda não estiver logado). Não é chamado ao iniciar o jogo:
    // FindSessions, QuickJoin e CreateSession chamam sozinhos quando precisam.
    UFUNCTION(BlueprintCallable, Category = "Multiplayer Sessions")
    void LoginOnline();

    UFUNCTION(BlueprintPure, Category = "Multiplayer Sessions")
    bool IsLoggingIn() const { return bLoginInProgress; }

    UFUNCTION(BlueprintPure, Category = "Multiplayer Sessions")
    bool IsSearchingSessions() const { return bIsSearching; }

    UFUNCTION(BlueprintPure, Category = "Multiplayer Sessions")
    float GetSearchCooldownRemaining() const;

    UPROPERTY(BlueprintAssignable, Category = "Multiplayer Sessions")
    FOnMultiplayerLoginStatusChanged OnLoginStatusChanged;

    // Resultado da busca online (FindSessions / QuickJoin).
    UPROPERTY(BlueprintAssignable, Category = "Multiplayer Sessions")
    FOnHNSessionsFound OnSessionsFound;

    // Resultado da busca LAN (FindLANSessions / QuickJoinLAN).
    UPROPERTY(BlueprintAssignable, Category = "Multiplayer Sessions")
    FOnHNSessionsFound OnLANSessionsFound;

    UPROPERTY(BlueprintAssignable, Category = "Multiplayer Sessions")
    FOnHNJoinFailed OnJoinFailed;

    //Delegates
    void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

protected:
    bool bIsLoggedIn = false;
	bool bIsLanSession = false;
    bool bIsSearching = false;
    bool bQuickJoinPending = false;
    double LastSearchStartTime = -1.0;

    // Level que o host vai abrir quando a criação da sala terminar.
    int32 PendingSeason = 1;
    int32 PendingStage = 1;

    FDelegateHandle PostLoadMapHandle;
    void OnPostLoadMap(UWorld* LoadedWorld);

    // Host: mantém a sala anunciada em dia (level atual + quantidade de jogadores).
    // O NULL (LAN) não conta o host nas vagas, então publicamos a contagem real como atributo.
    FDelegateHandle PostLoginHandle;
    FDelegateHandle LogoutHandle;
    FTSTicker::FDelegateHandle SessionSyncTickerHandle;
    void OnGameModePostLogin(class AGameModeBase* GameMode, class APlayerController* NewPlayer);
    void OnGameModeLogout(class AGameModeBase* GameMode, class AController* Exiting);
    bool IsOurWorld(const UWorld* World) const;
    void RequestSessionSync();
    void SyncHostedSession();

    // Jogadores na sala: usa o atributo PlayerCount (publicado pelo host) ou, se não existir, as vagas do serviço.
    static void GetSessionPlayerCounts(const FOnlineSession& Session, int32& OutCurrent, int32& OutMax, bool& bOutFull);

    static FString MakeLevelMapPath(int32 Season, int32 Stage);
    static bool GetIntSetting(const FOnlineSessionSettings& Settings, FName Key, int32& OutValue);

    // FindSessions chamado antes do login terminar: a busca roda assim que o login concluir.
    bool bSearchPendingLogin = false;

    // Busca LAN em andamento (a online usa bIsSearching). As duas podem rodar ao mesmo tempo.
    bool bIsSearchingLAN = false;
    // A sala em HostFoundSession é LAN.
    bool bJoiningLan = false;

    // Interfaces usadas nas buscas e no join atuais (EOS ou NULL), pra limpar os delegates no lugar certo.
    IOnlineSessionPtr SearchSessionInterface;
    IOnlineSessionPtr LANSearchSessionInterface;
    IOnlineSessionPtr JoinSessionInterface;
    FDelegateHandle FindLANSessionsCompleteDelegateHandle;

    bool StartSearch(bool bLan, int32 MaxSearchResults);
    void OnFindLANSessionsComplete(bool bWasSuccessful);
    void FinishSearch(bool bLan, bool bWasSuccessful);
    void FailJoin(const FText& Reason);

    // Parâmetros do QuickJoin: se não achar sala, cria uma com estes valores.
    bool bQuickJoinLan = false;
    bool bQuickJoinCreateIfNone = true;
    int32 QuickCreateConnections = 4;
    int32 QuickCreateSeason = 1;
    int32 QuickCreateStage = 1;
    void HandleQuickJoinResults(const TArray<FHNSessionInfo>& Sessions);
    int32 PendingMaxSearchResults = 100;

    // Pega o OnlineSubsystem certo para este GameInstance (no PIE cada janela tem o seu).
    IOnlineSubsystem* GetOnlineSubsystem(FName ServiceName = NAME_None) const;

    void Login();
    void InitIdentity();
    void FinishLogin(bool bWasSuccessful, const FString& Error);
    bool bLoginInProgress = false;
    TFunction<void()> PendingAfterLogin;
    void DestroyExistingSessionThen(TFunction<void()> Next);
    TArray<FHNSessionInfo> BuildSessionList(bool bLan) const;
    static FString SanitizeServerName(const FString& InName);

    IOnlineSessionPtr SessionInterface;
    IOnlineIdentityPtr IdentityInterface;

    FOnLoginCompleteDelegate LoginCompleteDelegate;
    FDelegateHandle LoginCompleteDelegateHandle;
    FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
    FDelegateHandle CreateSessionCompleteDelegateHandle;
    FDelegateHandle FindSessionsCompleteDelegateHandle;
    FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
    FDelegateHandle JoinSessionCompleteDelegateHandle;
    FDelegateHandle DestroySessionCompleteDelegateHandle;

    IOnlineSessionPtr DestroyingSessionInterface;
    TFunction<void()> PendingAfterDestroy;

private:
    void PrintString(const FString& text);
};
