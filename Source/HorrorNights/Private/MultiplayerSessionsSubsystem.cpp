// Sunnymoon Softwares - All rights reserved


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"

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

    if (IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        bIsLoggedIn = true;
        OnLoginStatusChanged.Broadcast(true);
        return;
    }

    LoginCompleteDelegate = FOnLoginCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnLoginComplete);
    LoginCompleteDelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegate);

    IdentityInterface->AutoLogin(0);

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

    SessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineService);
    SessionSettings->BuildUniqueId = 1;

    if (!SessionInterface->CreateSession(*UserId, NAME_GameSession, *SessionSettings))
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    }
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
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
        UE_LOG(LogTemp, Warning, TEXT("Sessão EOS criada com sucesso!"));
        GetGameInstance()->GetWorld()->ServerTravel("/Game/Maps/Level_1-1?listen");
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Falha ao criar sessão EOS!"));
    }
}

void UMultiplayerSessionsSubsystem::CreateLANSession(int32 NumPublicConnections, FString MatchType)
{
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
        UE_LOG(LogTemp, Warning, TEXT("Encontrei %d sessões na nuvem!"), LastSessionSearch->SearchResults.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Nenhuma sessão encontrada ou falha na busca."));
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
