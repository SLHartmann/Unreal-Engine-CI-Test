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
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

struct LACAction {
    short type = 0;
    FString key = "";
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    double delay = 0.0;
    bool event = 0;

    //Constructor for keyboard actions
    LACAction(short t, FString k, double d, bool e) {
        type = t;
        key = k;
        delay = d;
        event = e;
    }

    //Constructor for mouse sequence
    LACAction(short t, float x, float y, double d) {
        type = t;
        mouseX = x;
        mouseY = y;
        delay = d;
    }

    //Constructor for delays
    LACAction(short t, double d) {
        type = t;
        delay = d;
    }
};

UWorld* GetTestWorld();
void readLACSequence(TArray<LACAction> &);



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

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSingleKeyPressDelay, FString, key, double, delay);
bool FSingleKeyPressDelay::Update() {
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(delay));
    ADD_LATENT_AUTOMATION_COMMAND(FPressOneKey(key));
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSingleKeyReleaseDelay, FString, key, double, delay);
bool FSingleKeyReleaseDelay::Update() {
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(delay));
    ADD_LATENT_AUTOMATION_COMMAND(FReleaseOneKey(key));
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FSingleKeyPressReleaseWithDelay, FString, key, double, delay_press, double, delay_release);
bool FSingleKeyPressReleaseWithDelay::Update() {
    ADD_LATENT_AUTOMATION_COMMAND(FPressOneKey(key));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(delay_press));
    ADD_LATENT_AUTOMATION_COMMAND(FReleaseOneKey(key));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(delay_release));
    return true;
}


DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSKPD, FString, key, double, delay);
bool FSKPD::Update() {
    const double NewTime = FPlatformTime::Seconds();
    if (NewTime - StartTime < delay)
    {
        return false;
    }
    APlayerController* pc = GetTestWorld()->GetFirstPlayerController();
    pc->InputKey(FKey(TCHAR_TO_ANSI(*key)), EInputEvent::IE_Pressed, 1.0f, false);
    //Wait for confirmation that the event was received and processed
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSKRD, FString, key, double, delay);
bool FSKRD::Update() {
    const double NewTime = FPlatformTime::Seconds();
    if (NewTime - StartTime < delay)
    {
        return false;
    }
    APlayerController* pc = GetTestWorld()->GetFirstPlayerController();
    pc->InputKey(FKey(TCHAR_TO_ANSI(*key)), EInputEvent::IE_Released, 1.0f, false);
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

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(FSingleAxisInputSmoothDelay, FString, key, float, value, float, duration, double, delay);
bool FSingleAxisInputSmoothDelay::Update() {
    ADD_LATENT_AUTOMATION_COMMAND(FAxisInputSmooth(key, value, duration));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(delay));
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(FTwoAxisInputSmoothDelay, float, valueX, float, valueY, float, duration, double, delay);
bool FTwoAxisInputSmoothDelay::Update() {
    ADD_LATENT_AUTOMATION_COMMAND(FTwoAxisInputSmooth(valueX, valueY, duration));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(delay));
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FTAIS, float, valueX, float, valueY, double, duration);
bool FTAIS::Update() {
    const double currentTime = FPlatformTime::Seconds();
    if (currentTime - StartTime < duration) {
        APlayerController* pc = GetTestWorld()->GetFirstPlayerController();
        float deltaTime = GetTestWorld()->DeltaTimeSeconds;

        if (!pc->InputComponent->GetAxisValue("Turn") != 0.0f) {
            pc->AddYawInput(valueX);
        }
        if (!pc->InputComponent->GetAxisValue("LookUp") != 0.0f) {
            pc->AddPitchInput(-valueY);
        }
        //pc->InputAxis(FKey("MouseX"), valueX, 1.0f, 1, false);
        //pc->InputAxis(FKey("MouseY"), -valueY, 1.0f, 1, false);
        //pc->AddYawInput(valueX);
        //pc->AddPitchInput(valueY);
        
        //ADD_LATENT_AUTOMATION_COMMAND(FAxisInput(key, value*deltaTime));
        return false;
    }
    APlayerController* pc = GetTestWorld()->GetFirstPlayerController();
    
    /* WORKS
    pc->SetControlRotation(FRotator(valueX, valueY, 0.0f));
    */
    
    //pc->AddYawInput(valueX);
    //pc->AddPitchInput(-valueY);
    return true;
}

/*
*                               Check Conditions
*/

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(FCheckPlayerPosition, bool*, ret, float, x, float, y, float, z);
bool FCheckPlayerPosition::Update() {
    APlayerController* pc = GetTestWorld()->GetFirstPlayerController();
    FVector loc = pc->GetPawn()->GetActorLocation();
    UE_LOG(LogTemp, Warning, TEXT("X should be %f, is %f"), x, loc.X);
    if (loc.X <= x-1 || loc.X >= x+1) {
        ret = false;
    }
    UE_LOG(LogTemp, Warning, TEXT("X should be %f, is %f"), y, loc.Y);
    if (loc.Y <= y - 1 || loc.Y >= y + 1) {
        ret = false;
    }
    UE_LOG(LogTemp, Warning, TEXT("X should be %f, is %f"), z, loc.Z);
    if (loc.Z <= z - 1 || loc.Z >= z + 1) {
        ret = false;
    }
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
    TArray<FString> keys;
    keys.Add("W");
    keys.Add("A");
    keys.Add("A");
    keys.Add("W");
    keys.Add("D");
    keys.Add("D");
    keys.Add("SpaceBar");
    keys.Add("D");
    keys.Add("SpaceBar");
    keys.Add("D");
    TArray<double> delays;
    delays.Add(0.225076);
    delays.Add(0.124971);
    delays.Add(1.383391);
    delays.Add(1.499978);
    delays.Add(0.058328);
    delays.Add(1.191685);
    delays.Add(0.316701);
    delays.Add(0.124966);
    delays.Add(0.024978);
    delays.Add(0.675);
    TArray<bool> types;
    types.Add(true);
    types.Add(true);
    types.Add(false);
    types.Add(false);
    types.Add(true);
    types.Add(false);
    types.Add(true);
    types.Add(true);
    types.Add(false);
    types.Add(false);
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.0f));

    //Execute action sequence
    for (int i = 0; i < keys.Num(); i++) {
        if (types[i]) {
            ADD_LATENT_AUTOMATION_COMMAND(FSKPD(keys[i], delays[i]));
        }
        else {
            ADD_LATENT_AUTOMATION_COMMAND(FSKRD(keys[i], delays[i]));
        }
    }
    
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyProjectPlayLACSequence, "TestGroup.My Project Play LAC Sequence", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ClientContext);
bool FMyProjectPlayLACSequence::RunTest(const FString& Parameters) {
    FString mapName = "/Game/FirstPersonCPP/Maps/FirstPersonExampleMap";
    AutomationOpenMap(mapName);
    TArray<LACAction> seq;
    readLACSequence(seq);
    for (int i = 0; i < seq.Num(); i++) {
        if (seq[i].type == 0) {
            //keyboard
            if (seq[i].event) {
                ADD_LATENT_AUTOMATION_COMMAND(FSKPD(seq[i].key, seq[i].delay));
            }
            else {
                ADD_LATENT_AUTOMATION_COMMAND(FSKRD(seq[i].key, seq[i].delay));
            }
        }
        else if (seq[i].type == 1) {
            //mouse
            ADD_LATENT_AUTOMATION_COMMAND(FTAIS(seq[i].mouseX, seq[i].mouseY, seq[i].delay));
        }
        else if (seq[i].type == 2) {
            //delay
            ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(seq[i].delay));
        }
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyProjectRecordedSequence, "TestGroup.My Project Recorded LAC Sequence", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::ClientContext);
bool FMyProjectRecordedSequence::RunTest(const FString& Parameters) {
    //Init test map/wait for it to load
    FString mapName = "/Game/FirstPersonCPP/Maps/FirstPersonExampleMap";
    AutomationOpenMap(mapName);
    bool* ret = new bool;
    *ret = true;
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.0f));

    /** Insert LAC sequence below */
    ADD_LATENT_AUTOMATION_COMMAND(FSKPD("W", 0.22));
    ADD_LATENT_AUTOMATION_COMMAND(FSKPD("A", 0.12));
    ADD_LATENT_AUTOMATION_COMMAND(FSKRD("A", 1.38));
    ADD_LATENT_AUTOMATION_COMMAND(FSKRD("W", 1.49));
    ADD_LATENT_AUTOMATION_COMMAND(FSKPD("D", 0.05));
    ADD_LATENT_AUTOMATION_COMMAND(FSKRD("D", 1.19));
    ADD_LATENT_AUTOMATION_COMMAND(FSKPD("SpaceBar", 0.31));
    ADD_LATENT_AUTOMATION_COMMAND(FSKPD("D", 0.12));
    ADD_LATENT_AUTOMATION_COMMAND(FSKRD("SpaceBar", 0.02));
    ADD_LATENT_AUTOMATION_COMMAND(FSKRD("D", 0.67));
    /** Insert LAC sequence above */

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

void readLACSequence(TArray<LACAction> &actions) {
    FString file = FPaths::GameDevelopersDir();
    file.Append(TEXT("LACSequence.txt"));
    IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
    FString FileContent;
    if (FileManager.FileExists(*file))
    {
        if (FFileHelper::LoadFileToString(FileContent, *file, FFileHelper::EHashOptions::None))
        {
            TArray<FString> lines;
            //TArray<LACAction> actions;
            FileContent.ParseIntoArrayLines(lines);
            for (int i = 0; i < lines.Num(); i++) {
                FString* type = new FString(), * rest = new FString();
                lines[i].Split("|", type, rest);
                if (type->Equals("0")) {
                    //keyboard
                    FString* key = new FString(), * delay = new FString(), * event = new FString();
                    rest->Split("|", key, rest);
                    rest->Split("|", delay, event);
                    if (event->Equals("1")) {
                        actions.Add(LACAction(0, *key, FCString::Atof(*(*delay)), true));
                    }
                    else {
                        actions.Add(LACAction(0, *key, FCString::Atof(*(*delay)), false));
                    }
                }
                else if (type->Equals("1")) {
                    //mouse
                    FString* x = new FString(), * y = new FString(), * duration = new FString(), * rest2 = new FString();
                    rest->Split("|", x, rest);
                    rest->Split("|", y, duration);
                    actions.Add(LACAction(1, FCString::Atof(*(*x)), FCString::Atof(*(*y)), FCString::Atof(*(*duration))));
                }
                else if (type->Equals("2")) {
                    //delay
                    actions.Add(LACAction(2, FCString::Atof(*(*rest))));
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("FileManipulation: Did not load text from file"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FileManipulation: ERROR: Can not read the file because it was not found."));
        UE_LOG(LogTemp, Warning, TEXT("FileManipulation: Expected file location: %s"), *file);
    }
}