// Sunnymoon Softwares - All rights reserved


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Online/OnlineSessionNames.h"
#include "Misc/PackageName.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"

namespace HNSessionKeys
{
    static const FName MatchType(TEXT("MatchType"));
    static const FString MatchTypeValue(TEXT("Horror"));
    static const FName ServerName(TEXT("ServerName"));
    static const FName HostName(TEXT("HostName"));
    static const FName GameMode(TEXT("GameMode"));
    static const FName Season(TEXT("Season"));
    static const FName Stage(TEXT("Stage"));
    static const FName PlayerCount(TEXT("PlayerCount"));
}

void UMultiplayerSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Quando o host troca de mapa (ServerTravel, Open Level etc.), atualiza o level anunciado na sala.
    PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMultiplayerSessionsSubsystem::OnPostLoadMap);

    // Quando alguém entra ou sai da partida (no host), atualiza a contagem de jogadores da sala.
    PostLoginHandle = FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &UMultiplayerSessionsSubsystem::OnGameModePostLogin);
    LogoutHandle = FGameModeEvents::GameModeLogoutEvent.AddUObject(this, &UMultiplayerSessionsSubsystem::OnGameModeLogout);

    // Não abre o login da Epic ao iniciar o jogo (quem joga só LAN não precisa dele).
    // O login acontece na primeira ação online: FindSessions, QuickJoin, CreateSession ou LoginOnline.
    InitIdentity();
}

void UMultiplayerSessionsSubsystem::InitIdentity()
{
    IOnlineSubsystem* Subsystem = GetOnlineSubsystem(TEXT("EOS"));
    if (!Subsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("InitIdentity: OnlineSubsystem EOS não encontrado (verifique DefaultEngine.ini)."));
        return;
    }

    IdentityInterface = Subsystem->GetIdentityInterface();

    // Já logado (ex: sessão anterior ainda válida): marca sem abrir nada.
    if (IdentityInterface.IsValid() && IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        bIsLoggedIn = true;
    }
}

void UMultiplayerSessionsSubsystem::LoginOnline()
{
    if (bIsLoggedIn)
    {
        OnLoginStatusChanged.Broadcast(true);
        return;
    }

    if (bLoginInProgress)
    {
        return;
    }

    Login();
}

void UMultiplayerSessionsSubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
    FGameModeEvents::GameModePostLoginEvent.Remove(PostLoginHandle);
    FGameModeEvents::GameModeLogoutEvent.Remove(LogoutHandle);

    if (SessionSyncTickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(SessionSyncTickerHandle);
        SessionSyncTickerHandle.Reset();
    }

    Super::Deinitialize();
}

bool UMultiplayerSessionsSubsystem::IsOurWorld(const UWorld* World) const
{
    // No PIE várias janelas rodam no mesmo processo: só reage ao mundo deste GameInstance.
    return World && World->GetGameInstance() == GetGameInstance();
}

void UMultiplayerSessionsSubsystem::OnGameModePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
    if (GameMode && IsOurWorld(GameMode->GetWorld()))
    {
        RequestSessionSync();
    }
}

void UMultiplayerSessionsSubsystem::OnGameModeLogout(AGameModeBase* GameMode, AController* Exiting)
{
    if (GameMode && IsOurWorld(GameMode->GetWorld()))
    {
        RequestSessionSync();
    }
}

void UMultiplayerSessionsSubsystem::RequestSessionSync()
{
    // Junta várias mudanças seguidas (carregar mapa + login do host etc.) num único UpdateSession,
    // e espera um pouco pro jogador que saiu já ter sido removido do GameState.
    if (SessionSyncTickerHandle.IsValid())
    {
        return;
    }

    TWeakObjectPtr<UMultiplayerSessionsSubsystem> WeakThis(this);
    SessionSyncTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakThis](float)
    {
        if (UMultiplayerSessionsSubsystem* Self = WeakThis.Get())
        {
            Self->SessionSyncTickerHandle.Reset();
            Self->SyncHostedSession();
        }
        return false; // roda uma vez só
    }), 0.5f);
}

void UMultiplayerSessionsSubsystem::SyncHostedSession()
{
    UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
    if (!World || World->GetNetMode() == NM_Client)
    {
        return;
    }

    IOnlineSubsystem* Subsystem = bIsLanSession ? GetOnlineSubsystem(TEXT("NULL")) : GetOnlineSubsystem();
    IOnlineSessionPtr Sessions = Subsystem ? Subsystem->GetSessionInterface() : nullptr;
    if (!Sessions.IsValid())
    {
        return;
    }

    FNamedOnlineSession* NamedSession = Sessions->GetNamedSession(NAME_GameSession);
    if (!NamedSession || !NamedSession->bHosting)
    {
        return;
    }

    const int32 MaxPlayers = NamedSession->SessionSettings.NumPublicConnections;

    // O servidor recusa quem chegar além do limite da sala (padrão da engine seria 16).
    if (AGameModeBase* GameMode = World->GetAuthGameMode())
    {
        if (GameMode->GameSession && MaxPlayers > 0)
        {
            GameMode->GameSession->MaxPlayers = MaxPlayers;
        }
    }

    const AGameStateBase* GameState = World->GetGameState();
    const int32 PlayerCount = GameState ? GameState->PlayerArray.Num() : 1;

    int32 Season = 0, Stage = 0;
    const bool bHasLevel = ParseLevelName(World->GetMapName(), Season, Stage);

    int32 CurrentCount = -1, CurrentSeason = 0, CurrentStage = 0;
    GetIntSetting(NamedSession->SessionSettings, HNSessionKeys::PlayerCount, CurrentCount);
    GetIntSetting(NamedSession->SessionSettings, HNSessionKeys::Season, CurrentSeason);
    GetIntSetting(NamedSession->SessionSettings, HNSessionKeys::Stage, CurrentStage);

    const bool bCountChanged = CurrentCount != PlayerCount;
    const bool bLevelChanged = bHasLevel && (CurrentSeason != Season || CurrentStage != Stage);
    if (!bCountChanged && !bLevelChanged)
    {
        return;
    }

    FOnlineSessionSettings UpdatedSettings = NamedSession->SessionSettings;
    UpdatedSettings.Set(HNSessionKeys::PlayerCount, PlayerCount, EOnlineDataAdvertisementType::ViaOnlineService);
    if (bHasLevel)
    {
        UpdatedSettings.Set(HNSessionKeys::Season, Season, EOnlineDataAdvertisementType::ViaOnlineService);
        UpdatedSettings.Set(HNSessionKeys::Stage, Stage, EOnlineDataAdvertisementType::ViaOnlineService);
    }

    if (!Sessions->UpdateSession(NAME_GameSession, UpdatedSettings, true))
    {
        UE_LOG(LogTemp, Error, TEXT("SyncHostedSession: UpdateSession refused."));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("SyncHostedSession: room now advertises %d/%d players, level %d-%d."), PlayerCount, MaxPlayers, Season, Stage);
}

void UMultiplayerSessionsSubsystem::GetSessionPlayerCounts(const FOnlineSession& Session, int32& OutCurrent, int32& OutMax, bool& bOutFull)
{
    OutMax = Session.SessionSettings.NumPublicConnections;

    int32 PublishedCount = 0;
    if (GetIntSetting(Session.SessionSettings, HNSessionKeys::PlayerCount, PublishedCount) && PublishedCount > 0)
    {
        OutCurrent = FMath::Clamp(PublishedCount, 0, FMath::Max(OutMax, 0));
        bOutFull = OutMax > 0 && OutCurrent >= OutMax;
        return;
    }

    // Sala sem o atributo (versão antiga do jogo): usa as vagas do serviço.
    const int32 OpenSlots = Session.NumOpenPublicConnections;
    OutCurrent = FMath::Clamp(OutMax - OpenSlots, 0, FMath::Max(OutMax, 0));
    bOutFull = OpenSlots <= 0;
}

bool UMultiplayerSessionsSubsystem::GetIntSetting(const FOnlineSessionSettings& Settings, FName Key, int32& OutValue)
{
    // O EOS devolve inteiros como Int64 (mesmo gravando Int32), então Settings.Get(Key, int32&)
    // falha no cliente. Aqui aceitamos qualquer tipo numérico (ou string numérica).
    const FOnlineSessionSetting* Setting = Settings.Settings.Find(Key);
    if (!Setting)
    {
        return false;
    }

    const FVariantData& Data = Setting->Data;
    switch (Data.GetType())
    {
    case EOnlineKeyValuePairDataType::Int32:
    {
        int32 Value = 0;
        Data.GetValue(Value);
        OutValue = Value;
        return true;
    }
    case EOnlineKeyValuePairDataType::UInt32:
    {
        uint32 Value = 0;
        Data.GetValue(Value);
        OutValue = static_cast<int32>(Value);
        return true;
    }
    case EOnlineKeyValuePairDataType::Int64:
    {
        int64 Value = 0;
        Data.GetValue(Value);
        OutValue = static_cast<int32>(Value);
        return true;
    }
    case EOnlineKeyValuePairDataType::UInt64:
    {
        uint64 Value = 0;
        Data.GetValue(Value);
        OutValue = static_cast<int32>(Value);
        return true;
    }
    case EOnlineKeyValuePairDataType::Float:
    {
        float Value = 0.f;
        Data.GetValue(Value);
        OutValue = FMath::RoundToInt(Value);
        return true;
    }
    case EOnlineKeyValuePairDataType::Double:
    {
        double Value = 0.0;
        Data.GetValue(Value);
        OutValue = static_cast<int32>(FMath::RoundToDouble(Value));
        return true;
    }
    case EOnlineKeyValuePairDataType::String:
    {
        FString Value;
        Data.GetValue(Value);
        if (Value.IsNumeric())
        {
            OutValue = FCString::Atoi(*Value);
            return true;
        }
        return false;
    }
    default:
        return false;
    }
}

IOnlineSubsystem* UMultiplayerSessionsSubsystem::GetOnlineSubsystem(FName ServiceName) const
{
#if WITH_EDITOR
    // No PIE cada janela tem sua própria instância do OnlineSubsystem (ex: "EOS:Context_1").
    // IOnlineSubsystem::Get() puro devolve a instância global, que não é a usada pelo NetDriver
    // do mundo — o login acontecia numa instância e o listen do mapa em outra (sem usuário).
    if (const UGameInstance* GameInstance = GetGameInstance())
    {
        if (const FWorldContext* WorldContext = GameInstance->GetWorldContext())
        {
            if (IOnlineSubsystemUtils* Utils = Online::GetUtils())
            {
                return IOnlineSubsystem::Get(Utils->GetOnlineIdentifier(*WorldContext, ServiceName));
            }
        }
    }
#endif

    return IOnlineSubsystem::Get(ServiceName);
}

FString UMultiplayerSessionsSubsystem::MakeLevelMapPath(int32 Season, int32 Stage)
{
    return FString::Printf(TEXT("/Game/Maps/Level_%d-%d"), Season, Stage);
}

bool UMultiplayerSessionsSubsystem::ParseLevelName(const FString& MapName, int32& Season, int32& Stage)
{
    Season = 0;
    Stage = 0;

    // Aceita "Level_1-2", "/Game/Maps/Level_1-2", "/Game/Maps/Level_1-2.Level_1-2" e "UEDPIE_0_Level_1-2".
    FString Name = FPackageName::GetShortName(MapName);
    int32 DotIndex;
    if (Name.FindChar(TEXT('.'), DotIndex))
    {
        Name.LeftInline(DotIndex);
    }

    const int32 PrefixIndex = Name.Find(TEXT("Level_"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
    if (PrefixIndex == INDEX_NONE)
    {
        return false;
    }

    FString SeasonStr, StageStr;
    if (!Name.Mid(PrefixIndex + 6).Split(TEXT("-"), &SeasonStr, &StageStr))
    {
        return false;
    }

    if (SeasonStr.IsEmpty() || StageStr.IsEmpty() || !SeasonStr.IsNumeric() || !StageStr.IsNumeric())
    {
        return false;
    }

    const int32 ParsedSeason = FCString::Atoi(*SeasonStr);
    const int32 ParsedStage = FCString::Atoi(*StageStr);
    if (ParsedSeason <= 0 || ParsedStage <= 0)
    {
        return false;
    }

    Season = ParsedSeason;
    Stage = ParsedStage;
    return true;
}

bool UMultiplayerSessionsSubsystem::GetCurrentLevel(int32& Season, int32& Stage) const
{
    const UWorld* World = GetWorld();
    if (!World)
    {
        Season = 0;
        Stage = 0;
        return false;
    }

    return ParseLevelName(World->GetMapName(), Season, Stage);
}

void UMultiplayerSessionsSubsystem::OnPostLoadMap(UWorld* LoadedWorld)
{
    if (!LoadedWorld || LoadedWorld->GetNetMode() == NM_Client)
    {
        return;
    }

    if (IsOurWorld(LoadedWorld))
    {
        // Atualiza level e jogadores juntos (num único UpdateSession).
        RequestSessionSync();
    }
}

bool UMultiplayerSessionsSubsystem::UpdateSessionLevel(int32 Season, int32 Stage)
{
    if (Season <= 0 || Stage <= 0)
    {
        return false;
    }

    IOnlineSubsystem* Subsystem = bIsLanSession ? GetOnlineSubsystem(TEXT("NULL")) : GetOnlineSubsystem();
    if (!Subsystem)
    {
        return false;
    }

    IOnlineSessionPtr Sessions = Subsystem->GetSessionInterface();
    if (!Sessions.IsValid())
    {
        return false;
    }

    // Só o host pode alterar a sala.
    FNamedOnlineSession* NamedSession = Sessions->GetNamedSession(NAME_GameSession);
    if (!NamedSession || !NamedSession->bHosting)
    {
        return false;
    }

    int32 CurrentSeason = 0, CurrentStage = 0;
    GetIntSetting(NamedSession->SessionSettings, HNSessionKeys::Season, CurrentSeason);
    GetIntSetting(NamedSession->SessionSettings, HNSessionKeys::Stage, CurrentStage);
    if (CurrentSeason == Season && CurrentStage == Stage)
    {
        return true;
    }

    FOnlineSessionSettings UpdatedSettings = NamedSession->SessionSettings;
    UpdatedSettings.Set(HNSessionKeys::Season, Season, EOnlineDataAdvertisementType::ViaOnlineService);
    UpdatedSettings.Set(HNSessionKeys::Stage, Stage, EOnlineDataAdvertisementType::ViaOnlineService);

    if (!Sessions->UpdateSession(NAME_GameSession, UpdatedSettings, true))
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateSessionLevel: UpdateSession refused (level %d-%d)."), Season, Stage);
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("UpdateSessionLevel: session now advertises level %d-%d."), Season, Stage);
    return true;
}

void UMultiplayerSessionsSubsystem::Login()
{
    IOnlineSubsystem* Subsystem = GetOnlineSubsystem(TEXT("EOS"));
    if (!Subsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Login: OnlineSubsystem não encontrado (verifique DefaultEngine.ini)."));
        FinishLogin(false, TEXT("No EOS subsystem"));
        return;
    }

    IdentityInterface = Subsystem->GetIdentityInterface();
    if (!IdentityInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Login: IdentityInterface inválido."));
        FinishLogin(false, TEXT("No identity interface"));
        return;
    }

    if (IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        FinishLogin(true, FString());
        return;
    }

    LoginCompleteDelegate = FOnLoginCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnLoginComplete);
    LoginCompleteDelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegate);


    FOnlineAccountCredentials Credentials;
    Credentials.Type = TEXT("accountportal");
    Credentials.Id = TEXT("");
    Credentials.Token = TEXT("");

    UE_LOG(LogTemp, Warning, TEXT("Login: abrindo o login da Epic..."));
    bLoginInProgress = true;

    if (!IdentityInterface->Login(0, Credentials))
    {
        UE_LOG(LogTemp, Error, TEXT("LoginOnline: A função Login recusou a execução imediata."));
        IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegateHandle);
        bLoginInProgress = false;
        FinishLogin(false, TEXT("Login refused"));
    }
}

void UMultiplayerSessionsSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
    if (IdentityInterface.IsValid())
    {
        IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegateHandle);
    }

    FinishLogin(bWasSuccessful, Error);
}

void UMultiplayerSessionsSubsystem::FinishLogin(bool bWasSuccessful, const FString& Error)
{
    bIsLoggedIn = bWasSuccessful;
    bLoginInProgress = false;

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("Login no EOS concluído com sucesso!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Falha no login EOS: %s"), *Error);
    }

    OnLoginStatusChanged.Broadcast(bWasSuccessful);

    if (bSearchPendingLogin)
    {
        bSearchPendingLogin = false;

        if (bWasSuccessful)
        {
            FindSessions(PendingMaxSearchResults);
        }
        else
        {
            bQuickJoinPending = false;
            OnSessionsFound.Broadcast(TArray<FHNSessionInfo>(), false);
        }
    }

    // Ação online que estava esperando o login (ex: CreateSession).
    TFunction<void()> Pending = MoveTemp(PendingAfterLogin);
    PendingAfterLogin.Reset();
    if (bWasSuccessful && Pending)
    {
        Pending();
    }
}

void UMultiplayerSessionsSubsystem::DestroyExistingSessionThen(TFunction<void()> Next)
{
    if (!SessionInterface.IsValid() || SessionInterface->GetNamedSession(NAME_GameSession) == nullptr)
    {
        Next();
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Already had a local GameSession, destroying before continuing..."));

    DestroyingSessionInterface = SessionInterface;
    PendingAfterDestroy = MoveTemp(Next);
    DestroySessionCompleteDelegateHandle = DestroyingSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
        FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnDestroySessionComplete));

    if (!DestroyingSessionInterface->DestroySession(NAME_GameSession))
    {
        UE_LOG(LogTemp, Error, TEXT("Refuses to call DestroySession, the pending action was cancelled."));
        DestroyingSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
        DestroyingSessionInterface.Reset();
        PendingAfterDestroy.Reset();
    }
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (DestroyingSessionInterface.IsValid())
    {
        DestroyingSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
        DestroyingSessionInterface.Reset();
    }

    if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("DestroySession finish with failure, trying to continue anyway."));
    }

    TFunction<void()> Next = MoveTemp(PendingAfterDestroy);
    PendingAfterDestroy.Reset();

    if (Next)
    {
        Next();
    }
}

FString UMultiplayerSessionsSubsystem::SanitizeServerName(const FString& InName)
{
    FString Clean;
    Clean.Reserve(InName.Len());

    for (const TCHAR Char : InName)
    {
        if (Char >= 32 && Char != 127)
        {
            Clean.AppendChar(Char);
        }
    }

    Clean.TrimStartAndEndInline();
    return Clean.Left(MaxServerNameLength);
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType, FString ServerName, int32 Season, int32 Stage)
{
    bIsLanSession = false;
    PendingSeason = FMath::Max(1, Season);
    PendingStage = FMath::Max(1, Stage);

    if (!bIsLoggedIn)
    {
        // Faz o login primeiro e cria a sala assim que ele terminar.
        UE_LOG(LogTemp, Warning, TEXT("CreateSession: ainda não logado no EOS, a sala será criada após o login."));
        PendingAfterLogin = [this, NumPublicConnections, MatchType, ServerName, Season, Stage]()
        {
            CreateSession(NumPublicConnections, MatchType, ServerName, Season, Stage);
        };
        LoginOnline();
        return;
    }

    IOnlineSubsystem* Subsystem = GetOnlineSubsystem();
    if (!Subsystem)
        return;

    SessionInterface = Subsystem->GetSessionInterface();
    if (!SessionInterface.IsValid()) return;

    IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface();
    if (!IdentityPtr.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("CreateSession: IdentityInterface inválida."));
        return;
    }

    const TSharedPtr<const FUniqueNetId> UserId = IdentityPtr->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("CreateSession: usuário ainda não está logado no EOS (UserId inválido)."));
        return;
    }

    const FString HostName = IdentityPtr->GetPlayerNickname(*UserId);
    const FString CleanServerName = SanitizeServerName(ServerName);
    UE_LOG(LogTemp, Warning, TEXT("CreateSession: ServerName='%s' Host='%s' Level=%d-%d"), *CleanServerName, *HostName, PendingSeason, PendingStage);
    const bool isLan = (Subsystem->GetSubsystemName() == "NULL");

    DestroyExistingSessionThen([this, UserId, NumPublicConnections, MatchType, HostName, CleanServerName, isLan]()
    {
        if (!SessionInterface.IsValid()) return;

        CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionComplete);
        CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

        TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
        SessionSettings->bIsLANMatch = isLan;
        SessionSettings->NumPublicConnections = NumPublicConnections;
        SessionSettings->bAllowJoinInProgress = true;
        SessionSettings->bAllowJoinViaPresence = !isLan;
        SessionSettings->bShouldAdvertise = true;
        SessionSettings->bUsesPresence = !isLan;
        SessionSettings->bUseLobbiesIfAvailable = !isLan;

        SessionSettings->Set(HNSessionKeys::MatchType, HNSessionKeys::MatchTypeValue, EOnlineDataAdvertisementType::ViaOnlineService);
        SessionSettings->Set(HNSessionKeys::GameMode, MatchType, EOnlineDataAdvertisementType::ViaOnlineService);
        SessionSettings->Set(HNSessionKeys::HostName, HostName, EOnlineDataAdvertisementType::ViaOnlineService);
        SessionSettings->Set(HNSessionKeys::ServerName, CleanServerName, EOnlineDataAdvertisementType::ViaOnlineService);
        SessionSettings->Set(HNSessionKeys::Season, PendingSeason, EOnlineDataAdvertisementType::ViaOnlineService);
        SessionSettings->Set(HNSessionKeys::Stage, PendingStage, EOnlineDataAdvertisementType::ViaOnlineService);
        SessionSettings->Set(HNSessionKeys::PlayerCount, 1, EOnlineDataAdvertisementType::ViaOnlineService); // o host
        SessionSettings->BuildUniqueId = 1;

        if (!SessionInterface->CreateSession(*UserId, NAME_GameSession, *SessionSettings))
        {
            UE_LOG(LogTemp, Error, TEXT("CreateSession: Failed to call CreateSession on the interface."));
            SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        }
    });
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    IOnlineSubsystem* Subsystem = bIsLanSession ? GetOnlineSubsystem(TEXT("NULL")) : GetOnlineSubsystem();

    if (Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        }
    }

    if (bWasSuccessful)
    {
        const FString MapPath = MakeLevelMapPath(PendingSeason, PendingStage);

        if (bIsLanSession)
        {
            UE_LOG(LogTemp, Warning, TEXT("Sessão LAN criada com sucesso. Abrindo %s via rede local..."), *MapPath);
            // O segredo do LAN: passamos ?bIsLanMatch=1 para a Unreal não exigir a Epic
            GetGameInstance()->GetWorld()->ServerTravel(MapPath + TEXT("?listen?bIsLanMatch=1"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Sessão EOS criada com sucesso. Abrindo %s..."), *MapPath);
            if (IdentityInterface.IsValid())
            {
                TSharedPtr<const FUniqueNetId> UserId = IdentityInterface->GetUniquePlayerId(0);
                if (UserId.IsValid())
                {
                    FString TravelURL = FString::Printf(TEXT("%s?listen?EpicUsername=%s"), *MapPath, *UserId->ToString());
                    GetGameInstance()->GetWorld()->ServerTravel(TravelURL);
                    return;
                }
            }

            // Fallback caso a Identity falhe
            GetGameInstance()->GetWorld()->ServerTravel(MapPath + TEXT("?listen"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Falha ao criar sessão!"));
    }
}

void UMultiplayerSessionsSubsystem::CreateLANSession(int32 NumPublicConnections, FString MatchType, FString ServerName, int32 Season, int32 Stage)
{
	bIsLanSession = true;
    PendingSeason = FMath::Max(1, Season);
    PendingStage = FMath::Max(1, Stage);

    IOnlineSubsystem* Subsystem = GetOnlineSubsystem(TEXT("NULL"));
    if (!Subsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateLANSession: Subsistema NULL não encontrado."));
        return;
    }

    SessionInterface = Subsystem->GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("CreateLANSession: SessionInterface inválido."));
        return;
    }

    APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateLANSession: PlayerController inválido."));
        return;
    }

    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateLANSession: LocalPlayer inválido."));
        return;
    }

    TSharedPtr<const FUniqueNetId> UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();

    if (!UserId.IsValid())
    {
        UserId = Subsystem->GetIdentityInterface()->GetUniquePlayerId(0);
    }

    if (!UserId.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("CreateLANSession: Impossível obter um UniqueNetId válido para LAN."));
        return;
    }

    // Nome do host: o nick da Epic se estiver logado, senão o nome do computador.
    FString HostName;
    if (bIsLoggedIn && IdentityInterface.IsValid())
    {
        HostName = IdentityInterface->GetPlayerNickname(0);
    }
    if (HostName.IsEmpty())
    {
        HostName = FPlatformProcess::ComputerName();
    }

    const FString CleanServerName = SanitizeServerName(ServerName);
    UE_LOG(LogTemp, Warning, TEXT("CreateLANSession: ServerName='%s' Host='%s' Level=%d-%d"), *CleanServerName, *HostName, PendingSeason, PendingStage);

    DestroyExistingSessionThen([this, UserId, NumPublicConnections, MatchType, HostName, CleanServerName]()
    {
        if (!SessionInterface.IsValid()) return;

        CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionComplete);
        CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

        TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
        SessionSettings->bIsLANMatch = true;
        SessionSettings->NumPublicConnections = NumPublicConnections;
        SessionSettings->bAllowJoinInProgress = true;
        SessionSettings->bAllowJoinViaPresence = false;
        SessionSettings->bShouldAdvertise = true;
        SessionSettings->bUsesPresence = false;
        SessionSettings->bUseLobbiesIfAvailable = false;

        // Mesmos atributos da sala online, pra lista LAN mostrar nome, host e level.
        // BuildUniqueId fica no padrão da engine: a busca LAN compara com a build de quem procura.
        SessionSettings->Set(HNSessionKeys::MatchType, HNSessionKeys::MatchTypeValue, EOnlineDataAdvertisementType::ViaOnlineService);
        SessionSettings->Set(HNSessionKeys::GameMode, MatchType, EOnlineDataAdvertisementType::ViaOnlineService);
        SessionSettings->Set(HNSessionKeys::HostName, HostName, EOnlineDataAdvertisementType::ViaOnlineService);
        SessionSettings->Set(HNSessionKeys::ServerName, CleanServerName, EOnlineDataAdvertisementType::ViaOnlineService);
        SessionSettings->Set(HNSessionKeys::Season, PendingSeason, EOnlineDataAdvertisementType::ViaOnlineService);
        SessionSettings->Set(HNSessionKeys::Stage, PendingStage, EOnlineDataAdvertisementType::ViaOnlineService);
        SessionSettings->Set(HNSessionKeys::PlayerCount, 1, EOnlineDataAdvertisementType::ViaOnlineService); // o host

        if (!SessionInterface->CreateSession(*UserId, NAME_GameSession, *SessionSettings))
        {
            UE_LOG(LogTemp, Error, TEXT("CreateLANSession: Falha ao chamar CreateSession na interface."));
            SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        }
    });
}

float UMultiplayerSessionsSubsystem::GetSearchCooldownRemaining() const
{
    if (LastSearchStartTime < 0.0)
    {
        return 0.f;
    }

    const double Elapsed = FPlatformTime::Seconds() - LastSearchStartTime;
    return static_cast<float>(FMath::Max(0.0, MinSecondsBetweenSearches - Elapsed));
}

bool UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
    if (!bIsLoggedIn)
    {
        // A tela Online costuma ser criada antes do login terminar: guarda o pedido e busca depois.
        UE_LOG(LogTemp, Warning, TEXT("FindSessions: ainda não logado no EOS, a busca vai rodar quando o login terminar."));
        bSearchPendingLogin = true;
        PendingMaxSearchResults = MaxSearchResults;
        LoginOnline();
        return true;
    }

    if (bIsSearching)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindSessions: Already has a search in progress."));
        return false;
    }

    if (GetSearchCooldownRemaining() > 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindSessions: wait %.1fs to search again."), GetSearchCooldownRemaining());
        return false;
    }

    return StartSearch(false, MaxSearchResults);
}

bool UMultiplayerSessionsSubsystem::FindLANSessions(int32 MaxSearchResults)
{
    if (bIsSearchingLAN)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindLANSessions: Already has a search in progress."));
        return false;
    }

    return StartSearch(true, MaxSearchResults);
}

bool UMultiplayerSessionsSubsystem::StartSearch(bool bLan, int32 MaxSearchResults)
{
    IOnlineSubsystem* Subsystem = bLan ? GetOnlineSubsystem(TEXT("NULL")) : GetOnlineSubsystem();
    if (!Subsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("StartSearch: OnlineSubsystem %s não encontrado."), bLan ? TEXT("NULL") : TEXT("padrão"));
        return false;
    }

    IOnlineSessionPtr Sessions = Subsystem->GetSessionInterface();
    if (!Sessions.IsValid())
    {
        return false;
    }

    // Online precisa do usuário EOS. LAN não precisa de login: usa o jogador local 0.
    TSharedPtr<const FUniqueNetId> UserId;
    if (!bLan)
    {
        IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface();
        if (IdentityPtr.IsValid())
        {
            UserId = IdentityPtr->GetUniquePlayerId(0);
        }

        if (!UserId.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("FindSessions: Invalid UserId."));
            return false;
        }
    }

    // Online e LAN têm busca, resultado e delegate separados: as telas Online e LAN podem
    // buscar ao mesmo tempo sem uma sobrescrever a lista da outra.
    TSharedPtr<FOnlineSessionSearch>& Search = bLan ? LastLANSessionSearch : LastSessionSearch;
    Search = MakeShareable(new FOnlineSessionSearch());
    Search->MaxSearchResults = FMath::Max(1, MaxSearchResults);
    Search->bIsLanQuery = bLan;
    Search->QuerySettings.Set(HNSessionKeys::MatchType, HNSessionKeys::MatchTypeValue, EOnlineComparisonOp::Equals);
    if (!bLan)
    {
        Search->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
    }

    FDelegateHandle& Handle = bLan ? FindLANSessionsCompleteDelegateHandle : FindSessionsCompleteDelegateHandle;
    Handle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(
        this, bLan ? &UMultiplayerSessionsSubsystem::OnFindLANSessionsComplete : &UMultiplayerSessionsSubsystem::OnFindSessionsComplete));

    if (bLan)
    {
        LANSearchSessionInterface = Sessions;
        bIsSearchingLAN = true;
    }
    else
    {
        SearchSessionInterface = Sessions;
        bIsSearching = true;
        // O cooldown só vale pro EOS (serviço online). A busca LAN é local e barata.
        LastSearchStartTime = FPlatformTime::Seconds();
    }

    const bool bStarted = bLan
        ? Sessions->FindSessions(0, Search.ToSharedRef())
        : Sessions->FindSessions(*UserId, Search.ToSharedRef());

    if (!bStarted)
    {
        UE_LOG(LogTemp, Error, TEXT("StartSearch: the interface refused the search (LAN: %d)."), bLan ? 1 : 0);
        Sessions->ClearOnFindSessionsCompleteDelegate_Handle(Handle);
        (bLan ? bIsSearchingLAN : bIsSearching) = false;
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("%s search started..."), bLan ? TEXT("LAN") : TEXT("Online"));
    return true;
}

bool UMultiplayerSessionsSubsystem::QuickJoinLAN(bool bCreateIfNoneFound, int32 NumPublicConnections, int32 Season, int32 Stage)
{
    bQuickJoinPending = true;
    bQuickJoinLan = true;
    bQuickJoinCreateIfNone = bCreateIfNoneFound;
    QuickCreateConnections = FMath::Max(1, NumPublicConnections);
    QuickCreateSeason = FMath::Max(1, Season);
    QuickCreateStage = FMath::Max(1, Stage);

    if (bIsSearchingLAN)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuickJoinLAN: busca LAN em andamento, usando o resultado dela."));
        return true;
    }

    if (!FindLANSessions())
    {
        bQuickJoinPending = false;
        return false;
    }

    return true;
}

bool UMultiplayerSessionsSubsystem::QuickJoin(int32 MaxSearchResults, bool bCreateIfNoneFound, int32 NumPublicConnections, int32 Season, int32 Stage)
{
    bQuickJoinPending = true;
    bQuickJoinLan = false;
    bQuickJoinCreateIfNone = bCreateIfNoneFound;
    QuickCreateConnections = FMath::Max(1, NumPublicConnections);
    QuickCreateSeason = FMath::Max(1, Season);
    QuickCreateStage = FMath::Max(1, Stage);

    // Já tem uma busca rodando: quando ela terminar, o QuickJoin usa o resultado.
    if (bIsSearching)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuickJoin: busca em andamento, usando o resultado dela."));
        return true;
    }

    // Busca recente (dentro do cooldown): usa o resultado que já temos em vez de recusar.
    if (bIsLoggedIn && GetSearchCooldownRemaining() > 0.f && LastSessionSearch.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("QuickJoin: usando o resultado da última busca."));
        HandleQuickJoinResults(BuildSessionList(false));
        return true;
    }

    if (!FindSessions(MaxSearchResults))
    {
        bQuickJoinPending = false;
        return false;
    }

    return true;
}

void UMultiplayerSessionsSubsystem::HandleQuickJoinResults(const TArray<FHNSessionInfo>& Sessions)
{
    bQuickJoinPending = false;

    // A lista já vem ordenada (quase cheias primeiro, cheias no fim): pega a primeira com vaga.
    const FHNSessionInfo* Best = Sessions.FindByPredicate([](const FHNSessionInfo& Info) { return !Info.bIsFull; });
    if (Best)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuickJoin%s: entrando em '%s'."), bQuickJoinLan ? TEXT("LAN") : TEXT(""), *Best->ServerName);
        JoinSessionByIndex(Best->ResultIndex, bQuickJoinLan);
        return;
    }

    if (bQuickJoinCreateIfNone)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuickJoin%s: nenhuma sala disponível, criando uma nova."), bQuickJoinLan ? TEXT("LAN") : TEXT(""));
        if (bQuickJoinLan)
        {
            CreateLANSession(QuickCreateConnections, HNSessionKeys::MatchTypeValue, FString(), QuickCreateSeason, QuickCreateStage);
        }
        else
        {
            CreateSession(QuickCreateConnections, HNSessionKeys::MatchTypeValue, FString(), QuickCreateSeason, QuickCreateStage);
        }
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("QuickJoin: nenhuma sala disponível."));
}

TArray<FHNSessionInfo> UMultiplayerSessionsSubsystem::BuildSessionList(bool bLan) const
{
    const TSharedPtr<FOnlineSessionSearch>& Search = bLan ? LastLANSessionSearch : LastSessionSearch;

    TArray<FHNSessionInfo> Sessions;
    if (!Search.IsValid())
    {
        return Sessions;
    }

    const TArray<FOnlineSessionSearchResult>& Results = Search->SearchResults;

    for (int32 Index = 0; Index < Results.Num(); ++Index)
    {
        const FOnlineSessionSearchResult& Result = Results[Index];
        if (!Result.IsValid())
        {
            continue;
        }

        const FOnlineSession& Session = Result.Session;

        int32 CurrentPlayers = 0, MaxPlayers = 0;
        bool bFull = false;
        GetSessionPlayerCounts(Session, CurrentPlayers, MaxPlayers, bFull);

        if (MaxPlayers <= 0)
        {
            continue;
        }

        // Online: sala cheia some da lista. LAN: aparece como "cheia", pra quem está na mesma
        // rede entender por que não consegue entrar na sala do amigo.
        if (bFull && !bLan)
        {
            continue;
        }

        FHNSessionInfo Info;
        Info.ResultIndex = Index;
        Info.MaxPlayers = MaxPlayers;
        Info.CurrentPlayers = CurrentPlayers;
        Info.bIsFull = bFull;
        Info.bIsLan = bLan;

        Session.SessionSettings.Get(HNSessionKeys::HostName, Info.HostName);
        if (Info.HostName.IsEmpty())
        {
            Info.HostName = Session.OwningUserName;
        }
        if (Info.HostName.IsEmpty())
        {
            Info.HostName = NSLOCTEXT("HNSessions", "UnknownHost", "Player").ToString();
        }

        Session.SessionSettings.Get(HNSessionKeys::ServerName, Info.ServerName);
        Info.ServerName = SanitizeServerName(Info.ServerName);
        if (Info.ServerName.IsEmpty())
        {
            Info.ServerName = FText::Format(NSLOCTEXT("HNSessions", "DefaultServerName", "{0}'s Room"), FText::FromString(Info.HostName)).ToString();
        }

        Session.SessionSettings.Get(HNSessionKeys::GameMode, Info.MatchType);

        GetIntSetting(Session.SessionSettings, HNSessionKeys::Season, Info.Season);
        GetIntSetting(Session.SessionSettings, HNSessionKeys::Stage, Info.Stage);
        if (Info.Season > 0 && Info.Stage > 0)
        {
            Info.LevelName = FString::Printf(TEXT("%d-%d"), Info.Season, Info.Stage);
        }

        // Debug: mostra o que realmente chegou do EOS para esta sala.
        FString RawAttributes;
        for (const TPair<FName, FOnlineSessionSetting>& Pair : Session.SessionSettings.Settings)
        {
            RawAttributes += FString::Printf(TEXT("%s=%s(%s) "), *Pair.Key.ToString(), *Pair.Value.Data.ToString(), EOnlineKeyValuePairDataType::ToString(Pair.Value.Data.GetType()));
        }
        UE_LOG(LogTemp, Log, TEXT("Session[%d] ServerName='%s' Host='%s' Level='%s' Players=%d/%d | Raw: %s"),
            Index, *Info.ServerName, *Info.HostName, *Info.LevelName, Info.CurrentPlayers, Info.MaxPlayers, *RawAttributes);

        Sessions.Add(MoveTemp(Info));
    }

    for (int32 i = Sessions.Num() - 1; i > 0; --i)
    {
        Sessions.Swap(i, FMath::RandRange(0, i));
    }

    // Quase cheias primeiro; cheias (só LAN) no fim da lista.
    Sessions.StableSort([](const FHNSessionInfo& A, const FHNSessionInfo& B)
    {
        if (A.bIsFull != B.bIsFull)
        {
            return !A.bIsFull;
        }
        return (A.MaxPlayers - A.CurrentPlayers) < (B.MaxPlayers - B.CurrentPlayers);
    });

    // Na LAN são poucas salas, mostra todas.
    if (!bLan && Sessions.Num() > MaxSessionsToDisplay)
    {
        Sessions.SetNum(MaxSessionsToDisplay);
    }

    return Sessions;
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    FinishSearch(false, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindLANSessionsComplete(bool bWasSuccessful)
{
    FinishSearch(true, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::FinishSearch(bool bLan, bool bWasSuccessful)
{
    IOnlineSessionPtr& Interface = bLan ? LANSearchSessionInterface : SearchSessionInterface;
    if (Interface.IsValid())
    {
        Interface->ClearOnFindSessionsCompleteDelegate_Handle(bLan ? FindLANSessionsCompleteDelegateHandle : FindSessionsCompleteDelegateHandle);
        Interface.Reset();
    }
    (bLan ? bIsSearchingLAN : bIsSearching) = false;

    const TArray<FHNSessionInfo> Sessions = bWasSuccessful ? BuildSessionList(bLan) : TArray<FHNSessionInfo>();

    UE_LOG(LogTemp, Warning, TEXT("%s Session Search completed (success: %d). %d session(s) available."),
        bLan ? TEXT("LAN") : TEXT("Online"), bWasSuccessful ? 1 : 0, Sessions.Num());

    // O QuickJoin pendente só usa a busca do mesmo tipo (online ou LAN).
    const bool bShouldQuickJoin = bQuickJoinPending && bQuickJoinLan == bLan;
    if (bShouldQuickJoin)
    {
        bQuickJoinPending = false;
    }

    if (bLan)
    {
        OnLANSessionsFound.Broadcast(Sessions, bWasSuccessful);
    }
    else
    {
        OnSessionsFound.Broadcast(Sessions, bWasSuccessful);
    }

    if (bShouldQuickJoin)
    {
        if (bWasSuccessful)
        {
            HandleQuickJoinResults(Sessions);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("QuickJoin: a busca falhou."));
        }
    }
}

bool UMultiplayerSessionsSubsystem::JoinRoom(const FHNSessionInfo& Room)
{
    return JoinSessionByIndex(Room.ResultIndex, Room.bIsLan);
}

bool UMultiplayerSessionsSubsystem::JoinSessionByIndex(int32 ResultIndex, bool bLan)
{
    if (bLan ? bIsSearchingLAN : bIsSearching)
    {
        UE_LOG(LogTemp, Warning, TEXT("JoinSessionByIndex: wait for the current search to complete."));
        return false;
    }

    const TSharedPtr<FOnlineSessionSearch>& Search = bLan ? LastLANSessionSearch : LastSessionSearch;
    if (!Search.IsValid() || !Search->SearchResults.IsValidIndex(ResultIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("JoinSessionByIndex: invalid index %d (the list might be outdated)."), ResultIndex);
        FailJoin(NSLOCTEXT("HNSessions", "JoinOutdated", "This room list is outdated. Refresh and try again."));
        return false;
    }

    const FOnlineSessionSearchResult& Result = Search->SearchResults[ResultIndex];
    int32 CurrentPlayers = 0, MaxPlayers = 0;
    bool bFull = false;
    GetSessionPlayerCounts(Result.Session, CurrentPlayers, MaxPlayers, bFull);
    if (bFull)
    {
        FailJoin(NSLOCTEXT("HNSessions", "JoinFull", "The room is full."));
        return false;
    }

    HostFoundSession = Result;
    bJoiningLan = bLan;
    JoinSession();
    return true;
}

void UMultiplayerSessionsSubsystem::FailJoin(const FText& Reason)
{
    UE_LOG(LogTemp, Error, TEXT("Join failed: %s"), *Reason.ToString());
    OnJoinFailed.Broadcast(Reason);
}

void UMultiplayerSessionsSubsystem::JoinSession()
{
    UE_LOG(LogTemp, Warning, TEXT("=== JOINING %s SESSION ==="), bJoiningLan ? TEXT("LAN") : TEXT("ONLINE"));

    const FText GenericError = NSLOCTEXT("HNSessions", "JoinGeneric", "Could not join the room.");

    if (!HostFoundSession.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("JoinSession: no session selected."));
        FailJoin(GenericError);
        return;
    }

    IOnlineSubsystem* Subsystem = bJoiningLan ? GetOnlineSubsystem(TEXT("NULL")) : GetOnlineSubsystem();
    if (!Subsystem) { UE_LOG(LogTemp, Error, TEXT("JoinSession: Subsystem nulo")); FailJoin(GenericError); return; }

    SessionInterface = Subsystem->GetSessionInterface();
    if (!SessionInterface.IsValid()) { UE_LOG(LogTemp, Error, TEXT("JoinSession: SessionInterface inválido")); FailJoin(GenericError); return; }

    // Online precisa do usuário EOS. LAN usa o jogador local 0 (não precisa de login).
    TSharedPtr<const FUniqueNetId> UserId;
    if (!bJoiningLan)
    {
        IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface();
        if (IdentityPtr.IsValid())
        {
            UserId = IdentityPtr->GetUniquePlayerId(0);
        }

        if (!UserId.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("JoinSession: UserId inválido"));
            FailJoin(NSLOCTEXT("HNSessions", "JoinNotLoggedIn", "You are not signed in to the online service."));
            return;
        }
    }

    const bool bLan = bJoiningLan;
    DestroyExistingSessionThen([this, UserId, bLan, GenericError]()
    {
        if (!SessionInterface.IsValid())
        {
            FailJoin(GenericError);
            return;
        }

        JoinSessionInterface = SessionInterface;
        JoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnJoinSessionComplete);
        JoinSessionCompleteDelegateHandle = JoinSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

        const bool bStarted = bLan
            ? JoinSessionInterface->JoinSession(0, NAME_GameSession, HostFoundSession)
            : JoinSessionInterface->JoinSession(*UserId, NAME_GameSession, HostFoundSession);

        if (!bStarted)
        {
            UE_LOG(LogTemp, Error, TEXT("JoinSession: A função JoinSession retornou FALSE! Falha ao iniciar."));
            JoinSessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
            JoinSessionInterface.Reset();
            FailJoin(GenericError);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("JoinSession disparado com sucesso, aguardando callback de conclusão..."));
        }
    });
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    IOnlineSessionPtr Sessions = JoinSessionInterface;
    JoinSessionInterface.Reset();

    if (Sessions.IsValid())
    {
        Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        SessionInterface = Sessions;
    }

    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("Join realizado com sucesso! Obtendo endereço de viagem..."));

        FString ConnectString;
        if (Sessions.IsValid() && Sessions->GetResolvedConnectString(SessionName, ConnectString))
        {
            APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
            if (PlayerController)
            {
                UE_LOG(LogTemp, Warning, TEXT("Viajando para %s"), *ConnectString);
                PlayerController->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
                return;
            }
        }

        FailJoin(NSLOCTEXT("HNSessions", "JoinNoAddress", "Could not connect to the room."));
        return;
    }

    UE_LOG(LogTemp, Error, TEXT("Falha ao entrar na sessão. Erro ID: %d"), static_cast<int32>(Result));

    switch (Result)
    {
    case EOnJoinSessionCompleteResult::SessionIsFull:
        FailJoin(NSLOCTEXT("HNSessions", "JoinFull", "The room is full."));
        break;
    case EOnJoinSessionCompleteResult::SessionDoesNotExist:
        FailJoin(NSLOCTEXT("HNSessions", "JoinGone", "This room no longer exists."));
        break;
    case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
        FailJoin(NSLOCTEXT("HNSessions", "JoinNoAddress", "Could not connect to the room."));
        break;
    case EOnJoinSessionCompleteResult::AlreadyInSession:
        FailJoin(NSLOCTEXT("HNSessions", "JoinAlreadyIn", "You are already in a room."));
        break;
    default:
        FailJoin(NSLOCTEXT("HNSessions", "JoinGeneric", "Could not join the room."));
        break;
    }
}

void UMultiplayerSessionsSubsystem::JoinServerByIP(FString Address)
{
    if (Address.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("IP Address is empty!"));
        return;
    }

    if (SessionInterface.IsValid())
    {
        SessionInterface->DestroySession(NAME_GameSession);
    }

    APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
    if (PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Conectando direto no IP: %s"), *Address);
        PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
    }
}

void UMultiplayerSessionsSubsystem::PrintString(const FString& text)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, text);
    }
}
