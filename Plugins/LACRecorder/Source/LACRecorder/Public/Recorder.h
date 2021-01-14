// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/InputSettings.h"
#include "InputCoreTypes.h"
#include "LACAction.h"

/**
 * 
 */
class LACRECORDER_API Recorder
{
public:
	Recorder();
	~Recorder();
	void BeginPlay(APlayerController *);
	void Tick();
	void EndPlay();
private:
	void GetAllBoundKeys();
	void RecordLACSequence(double);

	TArray<FString> boundKeys;
	TArray<LACAction> sequence;

	APlayerController* pc;

	float mouseXRec = 0.0f;
	float mouseYRec = 0.0f;
	double numTicks = 0.0;
};
