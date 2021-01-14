// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LACAction.h"
#include "GameFramework/PlayerInput.h"

/**
 * 
 */
class LACRECORDER_API Repeater
{
public:
	Repeater(FString, APlayerController*);
	~Repeater();
	void Prepare();
	void Tick();
protected:
	long numTicks = 0;
	long counter = 0;
	TArray<LACAction> actions;
	APlayerController* pc;
	FString fileName;
};
