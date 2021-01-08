#include "LACRecorder.h"
#include "Modules/ModuleManager.h"
#include "LACAction.h"
#include "Containers/UnrealString.h"
#include "Containers/Array.h"
#include "InputCoreTypes.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerController.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, LACRecorder);

LACRecorder::LACRecorder() {

}

void LACRecorder::BeginPlay(APlayerController* plcl) {
	pc = plcl;
	getAllBoundKeys();
}

void LACRecorder::Tick(double) {
	checkForRecording();
	if (recording) {
		double delta = FPlatformTime::Seconds();
		recordLACSequence(delta);
	}
}

void LACRecorder::getAllBoundKeys() {
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

void LACRecorder::recordLACSequence(double delta) {
	if (!mouseKeyboard) {
		//keyboard input
		UPlayerInput* pi = pc->PlayerInput;
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
		FVector2D mouseDelta;
		pc->GetInputMouseDelta(mouseDelta.X, mouseDelta.Y);

		UE_LOG(LogTemp, Warning, TEXT("x: %f, y: %f"), mouseDelta.X, mouseDelta.Y);
		mouseXRec += mouseDelta.X;
		mouseYRec += mouseDelta.Y;
		numTicks++;
	}
}

void LACRecorder::checkForRecording() {
	UPlayerInput* pi = pc->PlayerInput;
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
		FVector location = pc->GetPawn()->GetActorLocation();
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
			FRotator rot = pc->GetControlRotation();
			sequence.Add(LACAction(1, rot.Pitch, rot.Yaw, FPlatformTime::Seconds()));
			*/

			UE_LOG(LogTemp, Warning, TEXT("[RAW] mouseX: %f, mouseY: %f"), mouseXRec, mouseYRec);
			mouseXRec /= numTicks;
			mouseYRec /= numTicks;
			sequence.Add(LACAction(1, mouseXRec, mouseYRec, FPlatformTime::Seconds()));
			UE_LOG(LogTemp, Warning, TEXT("Num Ticks: %f"), numTicks);
			UE_LOG(LogTemp, Warning, TEXT("mouseX: %f, mouseY: %f"), mouseXRec, mouseYRec);

			float yaw = pc->InputYawScale;
			float pitch = pc->InputPitchScale;
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