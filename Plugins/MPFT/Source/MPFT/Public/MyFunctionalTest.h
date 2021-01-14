// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "Repeater.h"
#include "MyFunctionalTest.generated.h"

/**
 * 
 */
UCLASS()
class MPFT_API AMyFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	virtual void PrepareTest();
	virtual void StartTest();
	virtual void Tick(float);
protected:
	Repeater *repeater;
};
