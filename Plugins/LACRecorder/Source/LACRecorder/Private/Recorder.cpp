// Fill out your copyright notice in the Description page of Project Settings.


#include "Recorder.h"

Recorder::Recorder()
{
}

Recorder::~Recorder()
{
}

void Recorder::BeginPlay(APlayerController *plcl) {
	pc = plcl;
	GetAllBoundKeys();
	sequence.Add(LACAction(2, FPlatformTime::Seconds()));
}

void Recorder::Tick() {
	double delta = FPlatformTime::Seconds();
	RecordLACSequence(delta);
}

void Recorder::GetAllBoundKeys() {
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

void Recorder::RecordLACSequence(double delta) {
	UPlayerInput* pi = pc->PlayerInput;
	for (int i = 0; i < boundKeys.Num(); i++) {
		if (pi->WasJustPressed(FKey(TCHAR_TO_ANSI(*boundKeys[i])))) {
			UE_LOG(LogTemp, Warning, TEXT("Following key was just pressed at %f: %s"), numTicks, *boundKeys[i]);
			sequence.Add(LACAction(0, boundKeys[i], numTicks /*FPlatformTime::Seconds()*/, true));
		}
		else if (pi->WasJustReleased(FKey(TCHAR_TO_ANSI(*boundKeys[i])))) {
			UE_LOG(LogTemp, Warning, TEXT("Following key was just released at %f: %s"), numTicks, *boundKeys[i]);
			sequence.Add(LACAction(0, boundKeys[i], numTicks /*FPlatformTime::Seconds()*/, false));
		}
	}
	FVector2D mouseDelta;
	pc->GetInputMouseDelta(mouseDelta.X, mouseDelta.Y);
	if (mouseDelta.X != 0 || mouseDelta.Y != 0) {
		UE_LOG(LogTemp, Warning, TEXT("x: %f, y: %f"), mouseDelta.X, mouseDelta.Y);
		sequence.Add(LACAction(1, mouseDelta.X, mouseDelta.Y, numTicks));
	}
	numTicks++;
}

void Recorder::EndPlay() {
	TArray<FString> output;
	for (int i = 1; i < sequence.Num(); i++) {
		FString s;
		if (sequence[i].type == 0) {
			//keyboard
			float d = sequence[i].delay;// - sequence[i - 1].delay;
			if (sequence[i].event) {
				//press
				s = FString::FromInt(sequence[i].type) + "|" + sequence[i].key + "|" + FString::FromInt(d /*sequence[i].delay - sequence[i - 1].delay*/) + "|1";
			}
			else {
				//release
				s = FString::FromInt(sequence[i].type) + "|" + sequence[i].key + "|" + FString::FromInt(d /*sequence[i].delay - sequence[i - 1].delay*/) + "|0";
			}
		}
		else if (sequence[i].type == 1) {
			//mouse
			s = "1|" + FString::SanitizeFloat(sequence[i].mouseX) + "|" + FString::SanitizeFloat(sequence[i].mouseY) + "|" + FString::FromInt(sequence[i].delay);
		}
		output.Add(s);
	}
	FString FileName = pc->GetWorld()->GetMapName() + "_" + FString::SanitizeFloat(FPlatformTime::Seconds()) + ".txt";
	FString FilePath = FPaths::ConvertRelativePathToFull(FPaths::GameDevelopersDir() + FileName);
	FFileHelper::SaveStringArrayToFile(output, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
}
