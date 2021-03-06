// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyProjectCharacter.h"
#include "MyProjectProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "GameFramework/InputSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AMyProjectCharacter

AMyProjectCharacter::AMyProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void AMyProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	PrimaryActorTick.bCanEverTick = true;

	//Setup the LAC recording
	//getAllBoundKeys();
	recorder = new Recorder();
	recorder->BeginPlay(GetWorld()->GetFirstPlayerController());
		
	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMyProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMyProjectCharacter::OnFire);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMyProjectCharacter::OnResetVR);

	PlayerInputComponent->BindAction("AnyKey", IE_Pressed, this, &AMyProjectCharacter::DetectAnyKeyPress);
	PlayerInputComponent->BindAction("AnyKey", IE_Released, this, &AMyProjectCharacter::DetectAnyKeyRelease);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AMyProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyProjectCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyProjectCharacter::LookUpAtRate);
	//pic = PlayerInputComponent;
}

void AMyProjectCharacter::DetectAnyKeyPress(FKey key) {
	GEditor->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("key press: " + key.ToString()));
	//UE_LOG(LogTemp, Warning, TEXT("key press/release: %s"), *key.ToString());
}

void AMyProjectCharacter::DetectAnyKeyRelease(FKey key) {
	GEditor->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("key release: " + key.ToString()));
	//UE_LOG(LogTemp, Warning, TEXT("key press/release: %s"), *key.ToString());
}

void AMyProjectCharacter::OnFire()
{
	GEditor->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("Firing the weapon"));
	UE_LOG(LogTemp, Warning, TEXT("Firing the weapon"));
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<AMyProjectProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<AMyProjectProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AMyProjectCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMyProjectCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AMyProjectCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void AMyProjectCharacter::EndPlay(const EEndPlayReason::Type EndPlayInEditor) {
	recorder->EndPlay();
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AMyProjectCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void AMyProjectCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMyProjectCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMyProjectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMyProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AMyProjectCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AMyProjectCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AMyProjectCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AMyProjectCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}

void AMyProjectCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	/*checkForRecording();
	if (recording) {
		double delta = FPlatformTime::Seconds();
		recordLACSequence(delta);
	}*/
	recorder->Tick();
}

void AMyProjectCharacter::getAllBoundKeys() {
	TArray<FInputActionKeyMapping> actionMappings = GetDefault<UInputSettings>()->GetActionMappings();
	TArray<FInputAxisKeyMapping> axisMappings = GetDefault<UInputSettings>()->GetAxisMappings();
	for (int i = 0; i < actionMappings.Num(); i++) {
		FString key = actionMappings[i].Key.ToString();
		if (!actionMappings[i].Key.IsGamepadKey() || actionMappings[i].Key.IsMouseButton()) {
			UE_LOG(LogTemp, Warning, TEXT("Following key is bound to an action: %s"), *key);
			boundKeys.Add(key);
		}
	}
	for (int i = 0; i < axisMappings.Num(); i++) {
		FString key = axisMappings[i].Key.ToString();
		if (key != "MouseX" && key != "MouseY" && (!axisMappings[i].Key.IsGamepadKey() || axisMappings[i].Key.IsMouseButton())) {
			UE_LOG(LogTemp, Warning, TEXT("Following key is bound to an axis: %s"), *key);
			boundKeys.Add(key);
		}
	}
}

void AMyProjectCharacter::recordLACSequence(double delta) {
	if (!mouseKeyboard) {
		//keyboard input
		UPlayerInput* pi = GetWorld()->GetFirstPlayerController()->PlayerInput;
		if (movement) {
			//movement input / one keystroke at a time
			for (int i = 0; i < boundKeys.Num(); i++) {
				if (pi->WasJustPressed(FKey(TCHAR_TO_ANSI(*boundKeys[i])))) {
					UE_LOG(LogTemp, Warning, TEXT("Following key was just pressed at %f: %s"), delta, *boundKeys[i]);
					sequence.Add(LACAction(0, boundKeys[i], delta /*FPlatformTime::Seconds()*/, true));
				}
				if (pi->WasJustReleased(FKey(TCHAR_TO_ANSI(*boundKeys[i])))) {
					UE_LOG(LogTemp, Warning, TEXT("Following key was just released at %f: %s"), delta, *boundKeys[i]);
					sequence.Add(LACAction(0, boundKeys[i], delta /*FPlatformTime::Seconds()*/, false));
				}
			}
		}
		else {
			//multiple keystrokes at a time possible, should not be movement keys
			for (int i = 0; i < boundKeys.Num(); i++) {
				if (pi->WasJustPressed(FKey(TCHAR_TO_ANSI(*boundKeys[i])))) {
					UE_LOG(LogTemp, Warning, TEXT("Following key was just pressed at %f: %s"), delta, *boundKeys[i]);
					sequence.Add(LACAction(3, boundKeys[i], delta /*FPlatformTime::Seconds()*/, true));
				}
				if (pi->WasJustReleased(FKey(TCHAR_TO_ANSI(*boundKeys[i])))) {
					UE_LOG(LogTemp, Warning, TEXT("Following key was just released at %f: %s"), delta, *boundKeys[i]);
					sequence.Add(LACAction(3, boundKeys[i], delta /*FPlatformTime::Seconds()*/, false));
				}
			}
		}
	}
	else {
		//mouse input
		APlayerController *pc = GetWorld()->GetFirstPlayerController();
		FVector2D mouseDelta;
		pc->GetInputMouseDelta(mouseDelta.X, mouseDelta.Y);
		
		UE_LOG(LogTemp, Warning, TEXT("x: %f, y: %f"), mouseDelta.X, mouseDelta.Y);
		mouseXRec += mouseDelta.X;
		mouseYRec += mouseDelta.Y;
		numTicks++;
	}
}

void AMyProjectCharacter::checkForRecording() {
	UPlayerInput* pi = GetWorld()->GetFirstPlayerController()->PlayerInput;
	if (pi->WasJustReleased(FKey("NumPadNine")) && !recording) {
		//Start the recording
		recording = !recording;
		sequence.Add(LACAction(0, "", FPlatformTime::Seconds(), false));
		UE_LOG(LogTemp, Warning, TEXT("Recording started!"));
	}
	else if (pi->WasJustReleased(FKey("NumPadNine")) && recording) {
		//Done recording, convert sequence and save to file
		/*TODO
		* Different write operations for different LACAction types
		* Cut time to max 3 digits e.g. 0.123
		* Add 1sec delay as last LACAction to give PlayerController time to finish movement
		*/
		recording = !recording;
		UE_LOG(LogTemp, Warning, TEXT("Recording finished!"));
		UE_LOG(LogTemp, Warning, TEXT("Saving sequence to: %s"), *FPaths::GameDevelopersDir());
		TArray<FString> output;
		for (int i = 1; i < sequence.Num(); i++) {
			FString s;
			if (sequence[i].type == 0 || sequence[i].type == 3) {
				//keyboard
				float d = sequence[i].delay - sequence[i - 1].delay;
				//d = FMath::RoundHalfFromZero(d * 1000) / 1000;
				if (sequence[i].event) {
					//press
					s = FString::FromInt(sequence[i].type) + "|" + sequence[i].key + "|" + FString::SanitizeFloat(d /*sequence[i].delay - sequence[i - 1].delay*/) + "|1";
				}
				else {
					//release
					s = FString::FromInt(sequence[i].type) + "|" + sequence[i].key + "|" + FString::SanitizeFloat(d /*sequence[i].delay - sequence[i - 1].delay*/) + "|0";
				}
			}
			else if (sequence[i].type == 1) {
				//mouse
				s = "1|" + FString::SanitizeFloat(sequence[i].mouseX) + "|" + FString::SanitizeFloat(sequence[i].mouseY) + "|" + FString::SanitizeFloat(sequence[i].delay - sequence[i - 1].delay);
			}
			else if (sequence[i].type == 2) {
				//delay
				s = "2|" + FString::SanitizeFloat(sequence[i].delay - sequence[i - 1].delay);
			}
			output.Add(s);
		}
		output.Add(FString("2|1.0"));
		FVector location = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
		//output.Add(FString("ADD_LATENT_AUTOMATION_COMMAND(FCheckPlayerPosition(ret, " + FString::SanitizeFloat(location.X) + "f, " + FString::SanitizeFloat(location.Y) + "f, " + FString::SanitizeFloat(location.Z) + "f));"));
		FString FilePath = FPaths::ConvertRelativePathToFull(FPaths::GameDevelopersDir() + TEXT("LACSequence.txt"));
		FFileHelper::SaveStringArrayToFile(output, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
	}
	else if (pi->WasJustPressed(FKey("NumPadEight")) && recording) {
		mouseKeyboard = !mouseKeyboard;
		if (!mouseKeyboard) {
			//create mouse action
			UE_LOG(LogTemp, Warning, TEXT("Switched to keyboard/movement"));
			
			/* WORKS
			FRotator rot = GetWorld()->GetFirstPlayerController()->GetControlRotation();
			sequence.Add(LACAction(1, rot.Pitch, rot.Yaw, FPlatformTime::Seconds()));
			*/

			UE_LOG(LogTemp, Warning, TEXT("[RAW] mouseX: %f, mouseY: %f"), mouseXRec, mouseYRec);
			mouseXRec /= numTicks;
			mouseYRec /= numTicks;
			sequence.Add(LACAction(1, mouseXRec, mouseYRec, FPlatformTime::Seconds()));
			UE_LOG(LogTemp, Warning, TEXT("Num Ticks: %f"), numTicks);
			UE_LOG(LogTemp, Warning, TEXT("mouseX: %f, mouseY: %f"), mouseXRec, mouseYRec);

			float yaw = GetWorld()->GetFirstPlayerController()->InputYawScale;
			float pitch = GetWorld()->GetFirstPlayerController()->InputPitchScale;
			UE_LOG(LogTemp, Warning, TEXT("yaw scale: %f, pitch scale: %f"), yaw, pitch);
		}
		else {
			//reset mouse recording for new input
			mouseXRec = 0.0f;
			mouseYRec = 0.0f;
			numTicks = 0.0f;
			sequence.Add(LACAction(2, FPlatformTime::Seconds()));
			UE_LOG(LogTemp, Warning, TEXT("Switched to mouse"));
		}
	}
	else if (pi->WasJustPressed(FKey("NumPadSeven")) && recording) {
		//Record multiple keystrokes, should not be movement keys
		movement = !movement;
		if (!movement) {
			UE_LOG(LogTemp, Warning, TEXT("Switched to keyboard/multiple keystrokes"));
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Switched to keyboard/movement"));
		}
	
	}
}