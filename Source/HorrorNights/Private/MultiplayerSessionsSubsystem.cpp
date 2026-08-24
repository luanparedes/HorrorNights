// Sunnymoon Softwares - All rights reserved


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Online/OnlineSessionNames.h"

void UMultiplayerSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Login();
}

void UMultiplayerSessionsSubsystem::Login()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(TEXT("EOS"));
    if (!Subsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Login: OnlineSubsystem não encontrado (verifique DefaultEngine.ini)."));
        return;
    }

    IdentityInterface = Subsystem->GetIdentityInterface();
    if (!IdentityInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Login: IdentityInterface inválido."));
        return;
    }

    LoginCompleteDelegate = FOnLoginCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnLoginComplete);
    LoginCompleteDelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegate);

    if (IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        bIsLoggedIn = true;
        OnLoginStatusChanged.Broadcast(true);
        return;
    }


    FOnlineAccountCredentials Credentials;
    Credentials.Type = TEXT("accountportal"); // Abre o fluxo de login da conta real da Epic
    Credentials.Id = TEXT("");
    Credentials.Token = TEXT("");

    if (!IdentityInterface->Login(0, Credentials))
    {
        UE_LOG(LogTemp, Error, TEXT("LoginOnline: A função Login recusou a execução imediata."));
        IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegateHandle);
    }




    //IdentityInterface->AutoLogin(0);

    //TEST CODE, REMOVE AFTER TESTS PASSED
    
    //FOnlineAccountCredentials Credentials;
    //Credentials.Type = TEXT("developer"); // Diz para o EOS usar a Dev Auth Tool local
    //Credentials.Id = TEXT("localhost:12879"); // Endereço onde a Dev Auth Tool está rodando
    //Credentials.Token = TEXT("luansparedes"); // Nome do perfil que você criou na ferramenta
    //
    //if (!IdentityInterface->Login(0, Credentials))
    //{
    //    UE_LOG(LogTemp, Error, TEXT("LoginOnline: A função Login recusou a execução imediata."));
    //    IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegateHandle);
    //}

    //END TESTS
}

void UMultiplayerSessionsSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
    if (IdentityInterface.IsValid())
    {
        IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegateHandle);
    }

    bIsLoggedIn = bWasSuccessful;

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("Login no EOS concluído com sucesso!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Falha no login EOS: %s"), *Error);
    }

    // Avisa a UI (Blueprint) que pode liberar o botão de multiplayer agora.
    OnLoginStatusChanged.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
    bIsLanSession = false;

    if (!bIsLoggedIn)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateSession: ainda não logado no EOS. Espere OnLoginStatusChanged antes de chamar isso."));
        return;
    }

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
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

    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession != nullptr)
    {
        SessionInterface->DestroySession(NAME_GameSession);
    }

    CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionComplete);
    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

    bool isLan = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");

    TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
    SessionSettings->bIsLANMatch = isLan;
    SessionSettings->NumPublicConnections = NumPublicConnections;
    SessionSettings->bAllowJoinInProgress = true;
    SessionSettings->bAllowJoinViaPresence = !isLan;
    SessionSettings->bShouldAdvertise = true;
    SessionSettings->bUsesPresence = !isLan;
    SessionSettings->bUseLobbiesIfAvailable = !isLan;

    SessionSettings->Set(FName("MatchType"), FString("Horror"), EOnlineDataAdvertisementType::ViaOnlineService);
    SessionSettings->BuildUniqueId = 1;

    if (!SessionInterface->CreateSession(*UserId, NAME_GameSession, *SessionSettings))
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    }
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    // 1. Pega o subsistema CORRETO baseado no que tentamos criar
    IOnlineSubsystem* Subsystem = bIsLanSession ? IOnlineSubsystem::Get(TEXT("NULL")) : IOnlineSubsystem::Get();

    if (Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            // Limpa o delegate do lugar certo!
            SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        }
    }

    if (bWasSuccessful)
    {
        if (bIsLanSession)
        {
            UE_LOG(LogTemp, Warning, TEXT("Sessão LAN criada com sucesso. Abrindo mapa via rede local..."));
            // O segredo do LAN: passamos ?bIsLanMatch=1 para a Unreal não exigir a Epic
            GetGameInstance()->GetWorld()->ServerTravel("/Game/Maps/Level_1-1?listen?bIsLanMatch=1");
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Sessão EOS criada com sucesso. Exigindo autenticação Epic..."));
            if (IdentityInterface.IsValid())
            {
                TSharedPtr<const FUniqueNetId> UserId = IdentityInterface->GetUniquePlayerId(0);
                if (UserId.IsValid())
                {
                    FString TravelURL = FString::Printf(TEXT("/Game/Maps/Level_1-1?listen?EpicUsername=%s"), *UserId->ToString());
                    GetGameInstance()->GetWorld()->ServerTravel(TravelURL);
                    return;
                }
            }

            // Fallback caso a Identity falhe
            GetGameInstance()->GetWorld()->ServerTravel("/Game/Maps/Level_1-1?listen");
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Falha ao criar sessão!"));
    }
}

void UMultiplayerSessionsSubsystem::CreateLANSession(int32 NumPublicConnections, FString MatchType)
{
	bIsLanSession = true;

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(TEXT("NULL"));
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

    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession != nullptr)
    {
        SessionInterface->DestroySession(NAME_GameSession);
    }

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

    SessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineService);
    SessionSettings->BuildUniqueId = 1;

    if (!SessionInterface->CreateSession(*UserId, NAME_GameSession, *SessionSettings))
    {
        UE_LOG(LogTemp, Error, TEXT("CreateLANSession: Falha ao chamar CreateSession na interface."));
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    }
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
    if (!bIsLoggedIn)
    {
        UE_LOG(LogTemp, Error, TEXT("FindSessions: ainda não logado no EOS. Espere OnLoginStatusChanged antes de chamar isso."));
        return;
    }

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return;

    SessionInterface = Subsystem->GetSessionInterface();
    if (!SessionInterface.IsValid()) return;

    IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface();
    if (!IdentityPtr.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("FindSessions: IdentityInterface inválida."));
        return;
    }

    const TSharedPtr<const FUniqueNetId> UserId = IdentityPtr->GetUniquePlayerId(0);

    FindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnFindSessionsComplete);
    FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

    LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
    LastSessionSearch->MaxSearchResults = MaxSearchResults;
    LastSessionSearch->bIsLanQuery = false;

    LastSessionSearch->QuerySettings.Set(FName("MatchType"), FString("Horror"), EOnlineComparisonOp::Equals);
    LastSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

    if (!SessionInterface->FindSessions(*UserId, LastSessionSearch.ToSharedRef()))
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
    }
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return;

    SessionInterface = Subsystem->GetSessionInterface();
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
    }

    if (bWasSuccessful && LastSessionSearch.IsValid() && LastSessionSearch->SearchResults.Num() > 0)
    {
        // Salva a primeira sala encontrada na variável da classe
        HostFoundSession = LastSessionSearch->SearchResults[0];
        UE_LOG(LogTemp, Warning, TEXT("Sala salva com sucesso! Pronta para o Join."));

        JoinSession();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Nenhuma sessão encontrada ou falha na busca."));
    }
}

void UMultiplayerSessionsSubsystem::JoinSession()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENTRANDO NO JOIN SESSION ==="));

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) { UE_LOG(LogTemp, Error, TEXT("JoinSession: Subsystem nulo")); return; }

    SessionInterface = Subsystem->GetSessionInterface();
    if (!SessionInterface.IsValid()) { UE_LOG(LogTemp, Error, TEXT("JoinSession: SessionInterface inválido")); return; }

    // ---- FIX: limpa qualquer sessão "GameSession" fantasma antes de tentar entrar ----
    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("JoinSession: já existia uma GameSession local, destruindo antes de entrar."));
        SessionInterface->DestroySession(NAME_GameSession);
    }
    // -----------------------------------------------------------------------------

    IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface();
    if (!IdentityPtr.IsValid()) { UE_LOG(LogTemp, Error, TEXT("JoinSession: IdentityPtr inválido")); return; }

    const TSharedPtr<const FUniqueNetId> UserId = IdentityPtr->GetUniquePlayerId(0);
    if (!UserId.IsValid()) { UE_LOG(LogTemp, Error, TEXT("JoinSession: UserId inválido")); return; }

    JoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnJoinSessionComplete);
    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

    if (!SessionInterface->JoinSession(*UserId, NAME_GameSession, HostFoundSession))
    {
        UE_LOG(LogTemp, Error, TEXT("JoinSession: A função JoinSession retornou FALSE! Falha ao iniciar."));
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("JoinSession disparado com sucesso, aguardando callback de conclusão..."));
    }
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        }
    }

    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("Join realizado com sucesso! Obtendo endereço de viagem..."));

        FString ConnectString;
        if (SessionInterface.IsValid() && SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
        {
            APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
            if (PlayerController)
            {
                // Viaja automaticamente para o IP/Porta resolvido pelo EOS
                PlayerController->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Falha ao entrar na sessão. Erro ID: %d"), static_cast<int32>(Result));
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
