#include <Editor\UnrealEd\Public\Tests\AutomationEditorCommon.h>
#include <Runtime\Engine\Classes\Kismet\GameplayStatics.h>
#include "Engine/StaticMeshActor.h"
#include "UnrealClient.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include <CoreUObject.h>
#include "Containers/UnrealString.h"
#include "Tests/AutomationCommon.h"
#include "AutomationDriverCommon.h"
#include "AutomationDriverTypeDefs.h"
#include <MyProject/MyProjectCharacter.h>
#include <MyProject/MyProject.h>

UWorld* GetTestWorld();

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyProjectPlaceholderTest, "TestGroup.TestSubgroup.MyProject Placeholder Test", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ClientContext)

bool FMyProjectPlaceholderTest::RunTest(const FString& Parameters)
{
    // Make the test pass by returning true, or fail by returning false.
    return true;
}

/**
* Load a map given by mapName and get all StaticMeshActors. Check that all StaticMeshActors are currently visible.
*/
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyProjectCheckActorVisibilityTest, "TestGroup.TestSubgroup.MyProject Check Actor Visibility Test", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ClientContext)

bool FMyProjectCheckActorVisibilityTest::RunTest(const FString& Parameters) {
    FString mapName = "/Game/FirstPersonCPP/Maps/FirstPersonExampleMap";
    FAutomationEditorCommonUtils::LoadMap(mapName);
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GEditor->GetEditorWorldContext().World(), AStaticMeshActor::StaticClass(), FoundActors);
    //if (FoundActors.Num() != 21)
        //return false;
    for (int i = 0; i < FoundActors.Num(); i++) {
        USceneComponent* sComp = (USceneComponent*)FoundActors[i]->GetComponentByClass(USceneComponent::StaticClass());
        if (sComp != NULL) {
            if (sComp->GetVisibleFlag() != true) {
                const FString msg = "Following UStaticActor is not set to be visible: " + FoundActors[i]->GetName();
                UE_LOG(LogEditorAutomationTests, Error, TEXT("%s"), *msg);
                return false;
            }
        }
    }
    return true;
}

/**
* Check whether all meshes have a materials in their slots
*/
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FMyProjectCheckAllMeshesForMaterialsTest, "TestGroup.TestSubgroup.MyProject Check all meshes for materials", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ClientContext)

void FMyProjectCheckAllMeshesForMaterialsTest::GetTests(TArray<FString>& OutBeautifiedNames, TArray <FString>& OutTestCommands) const{
    //Get all static mesh assets from the Game/Content folder
    FAutomationEditorCommonUtils::CollectGameContentTestsByClass(UStaticMesh::StaticClass(), true, OutBeautifiedNames, OutTestCommands);
}

bool FMyProjectCheckAllMeshesForMaterialsTest::RunTest(const FString& Parameters) {
    //auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(*Parameters);
    UStaticMesh* mesh = (UStaticMesh*)StaticLoadObject(UStaticMesh::StaticClass(), NULL, *Parameters);
    bool ret = true;
    if (mesh != nullptr)
    {
        if (mesh->GetMaterial(0) == NULL) {
            ret = false;
        }
        for (int i = 0; i < mesh->StaticMaterials.Num(); i++) {
            if (mesh->GetMaterial(i) == NULL) {
                const FString msg_error = "The UStaticMesh " + mesh->GetName() + " has no material assigned in slot " + FString::FromInt(i) + "/" + mesh->StaticMaterials[i].MaterialSlotName.ToString();                
                UE_LOG(LogEditorAutomationTests, Error, TEXT("%s"), *msg_error);
                ret = false;
            }
        }
    }
    return true;
}

/**
* DEBUG Create a new map and keep it visible for 4 seconds 
*/
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyProjectCreateNewMapTest, "TestGroup.TestSubgroup.MyProject Create a new map", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ClientContext)

bool FMyProjectCreateNewMapTest::RunTest(const FString& Parameters) {
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(4.0f));
    return true;
}

/**
* Load a map, enter PIE mode and 
*/
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyProjectTestPIEAndInput, "TestGroup B.TestSubgroup.MyProject Test PIE and input", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ClientContext)

bool FMyProjectTestPIEAndInput::RunTest(const FString& Parameters) {
    FString mapName = "/Game/FirstPersonCPP/Maps/FirstPersonExampleMap";
    FAutomationEditorCommonUtils::LoadMap(mapName);
    //FAutomationEditorCommonUtils::RunPIE();
    
    TArray< TSharedRef<SWindow> > AllWindows;
    FSlateApplication::Get().GetAllVisibleWindowsOrdered(AllWindows);
    FSlateApplication::Get().ProcessWindowActivatedEvent(FWindowActivateEvent(FWindowActivateEvent::EA_Activate, AllWindows[0]));
    
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.0f));
    

    if(GEditor != nullptr){
        //if (GEditor->GetActiveViewport() == nullptr)
        if (GEditor->GetActiveViewport() == nullptr)
        {
            const FString msg = "GEditor does not point to a valid Viewport!";
            UE_LOG(LogEditorAutomationTests, Error, TEXT("%s"), *msg);
            return false;
        }
        
        TArray<FEditorViewportClient*> vps = GEditor->GetAllViewportClients();
        for (int i = 1; i < vps.Num(); i++) {
            //FViewportClient* client = GEditor->GetActiveViewport()->GetClient();
            //bool ret = vps[i]->InputKey(FInputKeyEventArgs(vps[i]->Viewport, 0, EKeys::LeftMouseButton, EInputEvent::IE_Pressed));
            bool ret = vps[i]->InputKey(vps[i]->Viewport, 0, EKeys::LeftMouseButton, EInputEvent::IE_Pressed);
            ret = vps[i]->InputKey(vps[i]->Viewport, 0, EKeys::LeftMouseButton, EInputEvent::IE_Released);
            ret = vps[i]->InputKey(vps[i]->Viewport, 0, EKeys::SpaceBar, EInputEvent::IE_Pressed);
            /*if (ret != true) {
                return false;
            }*/
        }
    }

    /*if (GEditor->GetPIEWorldContext()->World()->GetFirstPlayerController() == nullptr)
        return false;*/
    

    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(8.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());

    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND(FWaitForActorsToBeInitialized);
bool FWaitForActorsToBeInitialized::Update() {
    //ADD_LATENT_AUTOMATION_COMMAND(FEditorAutomationLogCommand(TEXT("Executing action now")))
    APlayerController* pc = GetTestWorld()->GetFirstPlayerController();
    bool ret = pc->InputKey(FKey("W"), EInputEvent::IE_Pressed, 1.0f, false);
    //if (ret != true)
        //return false;
    //pc->InputKey(FKey("SpaceBar"), EInputEvent::IE_Released, 1.0f, false);
    return true; // Command completed
}

DEFINE_LATENT_AUTOMATION_COMMAND(FPressJumpKey);
bool FPressJumpKey::Update() {
    //ADD_LATENT_AUTOMATION_COMMAND(FEditorAutomationLogCommand(TEXT("Executing action now")))
    APlayerController* pc = GetTestWorld()->GetFirstPlayerController();
    pc->InputKey(FKey("SpaceBar"), EInputEvent::IE_Pressed, 1.0f, false);
    pc->InputKey(FKey("LeftMouseButton"), EInputEvent::IE_Pressed, 1.0f, false);
    return true; // Command completed
}

/*############################################################################
*                     Custom latent commands for key input
*/

/*
*                               KEYBOARD INPUT
*/
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FPressOneKey, FString, key);
bool FPressOneKey::Update() {
    APlayerController* pc = GetTestWorld()->GetFirstPlayerController();
    pc->InputKey(FKey(TCHAR_TO_ANSI(*key)), EInputEvent::IE_Pressed, 1.0f, false);
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FReleaseOneKey, FString, key);
bool FReleaseOneKey::Update() {
    APlayerController* pc = GetTestWorld()->GetFirstPlayerController();
    pc->InputKey(FKey(TCHAR_TO_ANSI(*key)), EInputEvent::IE_Released, 1.0f, false);
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSingleKeyPressDelay, FString, key, float, delay);
bool FSingleKeyPressDelay::Update() {
    ADD_LATENT_AUTOMATION_COMMAND(FPressOneKey(key));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(delay));
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSingleKeyReleaseDelay, FString, key, float, delay);
bool FSingleKeyReleaseDelay::Update() {
    ADD_LATENT_AUTOMATION_COMMAND(FReleaseOneKey(key));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(delay));
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FSingleKeyPressReleaseWithDelay, FString, key, float, delay_press, float, delay_release);
bool FSingleKeyPressReleaseWithDelay::Update() {
    ADD_LATENT_AUTOMATION_COMMAND(FPressOneKey(key));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(delay_press));
    ADD_LATENT_AUTOMATION_COMMAND(FReleaseOneKey(key));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(delay_release));
    return true;
}

/*
*                               MOUSE INPUT
*/

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FAxisInput, FString, key, float, delta);
bool FAxisInput::Update() {
    APlayerController* pc = GetTestWorld()->GetFirstPlayerController();
    pc->InputAxis(FKey(TCHAR_TO_ANSI(*key)), delta, 1.0f, 1, false);
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FAxisInputSmooth, FString, key, float, value, float, duration);
bool FAxisInputSmooth::Update() {
    const double currentTime = FPlatformTime::Seconds();
    if (currentTime - StartTime < duration) {
        APlayerController* pc = GetTestWorld()->GetFirstPlayerController();
        float deltaTime = GetTestWorld()->DeltaTimeSeconds;
        pc->InputAxis(FKey(TCHAR_TO_ANSI(*key)), value*deltaTime, 1.0f, 1, false);
        //ADD_LATENT_AUTOMATION_COMMAND(FAxisInput(key, value*deltaTime));
        return false;
    }
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FTwoAxisInputSmooth, float, valueX, float, valueY, float, duration);
bool FTwoAxisInputSmooth::Update() {
    const double currentTime = FPlatformTime::Seconds();
    if (currentTime - StartTime < duration) {
        APlayerController* pc = GetTestWorld()->GetFirstPlayerController();
        float deltaTime = GetTestWorld()->DeltaTimeSeconds;
        pc->InputAxis(FKey("MouseX"), valueX * deltaTime, 1.0f, 1, false);
        pc->InputAxis(FKey("MouseY"), valueY * deltaTime, 1.0f, 1, false);
        //ADD_LATENT_AUTOMATION_COMMAND(FAxisInput(key, value*deltaTime));
        return false;
    }
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(FSingleAxisInputSmoothDelay, FString, key, float, value, float, duration, float, delay);
bool FSingleAxisInputSmoothDelay::Update() {
    ADD_LATENT_AUTOMATION_COMMAND(FAxisInputSmooth(key, value, duration));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(delay));
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(FTwoAxisInputSmoothDelay, float, valueX, float, valueY, float, duration, float, delay);
bool FTwoAxisInputSmoothDelay::Update() {
    ADD_LATENT_AUTOMATION_COMMAND(FTwoAxisInputSmooth(valueX, valueY, duration));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(delay));
    return true;
}

//############################################################################

/**
* Load a map, enter PIE mode and move the mouse
*/
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyProjectPIETest, "TestGroup.TestSubgroup.MyProject PIE Test", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ClientContext)
bool FMyProjectPIETest::RunTest(const FString& Parameters) {
    FString mapName = "/Game/FirstPersonCPP/Maps/FirstPersonExampleMap";
    AutomationOpenMap(mapName);
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FSingleAxisInputSmoothDelay("MouseX", 2057.5f, 1.0f, 2.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FSingleAxisInputSmoothDelay("MouseY", 100.0f, 2.0f, 2.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FTwoAxisInputSmoothDelay(-1028.725f, -100.0f, 2.0f, 1.0f));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyProjectLACTest, "TestGroup.MyProject LAC Test", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ClientContext)
bool FMyProjectLACTest::RunTest(const FString& Parameters) {
    //Init test map/wait for it to load
    FString mapName = "/Game/FirstPersonCPP/Maps/FirstPersonExampleMap";
    AutomationOpenMap(mapName);
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.0f));

    //Execute action sequence
    ADD_LATENT_AUTOMATION_COMMAND(FSingleKeyPressDelay("W", 1.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FSingleKeyReleaseDelay("W", 2.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FSingleKeyPressReleaseWithDelay("SpaceBar", 0.1f, 2.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FSingleKeyPressDelay("A", 0.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FSingleKeyPressDelay("S", 0.5f));
    ADD_LATENT_AUTOMATION_COMMAND(FSingleKeyPressReleaseWithDelay("LeftMouseButton", 0.2f, 0.4f));
    ADD_LATENT_AUTOMATION_COMMAND(FSingleKeyPressReleaseWithDelay("LeftMouseButton", 0.2f, 0.4f));
    ADD_LATENT_AUTOMATION_COMMAND(FSingleKeyReleaseDelay("S", 0.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FSingleKeyReleaseDelay("A", 2.0f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyProjectWorldTest, "TestGroup.MyProject World Test", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ClientContext)
bool FMyProjectWorldTest::RunTest(const FString& Parameters) {
    FString mapNameA = "/Game/FirstPersonCPP/Maps/FirstPersonExampleMap";
    FString mapNameB = "/Game/Maps/NewProjectTest";
    AutomationOpenMap(mapNameA);
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.0f));
    AutomationOpenMap(mapNameB);
    return true;
}

UWorld* GetTestWorld() {
    const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
    for (const FWorldContext& Context : WorldContexts) {
        if (((Context.WorldType == EWorldType::PIE) || (Context.WorldType == EWorldType::Game))
            && (Context.World() != nullptr)) {
            return Context.World();
        }
    }

    return nullptr;
}

bool ue4EditorInput() {
    //UGameInstance* GameInst = GetGameInstance();
    //UGameViewportClient* ViewportClient = GameInst->GetGameViewportClient();
    //FViewport* Viewport = ViewportClient->Viewport;
    FEditorViewportClient* ViewportClient = (FEditorViewportClient*)GEditor->GetActiveViewport()->GetClient();
    if (ViewportClient == NULL) {
        const FString msg_error = "The Viewport Client is null";
        UE_LOG(LogEditorAutomationTests, Error, TEXT("%s"), *msg_error);
        return false;
    }
    FViewport* Viewport = ViewportClient->Viewport;
    if (Viewport == NULL) {
        const FString msg_error = "The Viewport is null";
        UE_LOG(LogEditorAutomationTests, Error, TEXT("%s"), *msg_error);
        return false;
    }

    int32 ControllerId = 0; // or whatever controller id, could be a function param
    FName PressedKey = FName(TEXT("Escape")); // or whatever key, could be a function param
    FInputKeyEventArgs Args = FInputKeyEventArgs(
        Viewport,
        ControllerId,
        FKey(PressedKey),
        EInputEvent::IE_Pressed);

    if (ViewportClient->InputKey(Viewport, ControllerId, FKey(PressedKey), EInputEvent::IE_Pressed)) {
        return true;
    }
    else {
        const FString msg_error = "InputKey Method did not return true.";
        UE_LOG(LogEditorAutomationTests, Error, TEXT("%s"), *msg_error);
        return false;
    }
}