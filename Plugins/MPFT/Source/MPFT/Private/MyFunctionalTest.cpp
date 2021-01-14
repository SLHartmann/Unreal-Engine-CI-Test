// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFunctionalTest.h"

void AMyFunctionalTest::PrepareTest() {
	repeater = new Repeater("Test.txt", GetWorld()->GetFirstPlayerController());
	repeater->Prepare();
}

void AMyFunctionalTest::StartTest() {
	UE_LOG(LogTemp, Warning, TEXT("The test has started!"));
}

void AMyFunctionalTest::Tick(float DeltaTime) {
	repeater->Tick();
}