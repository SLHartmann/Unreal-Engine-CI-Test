// Fill out your copyright notice in the Description page of Project Settings.


#include "Repeater.h"

Repeater::Repeater(FString fN, APlayerController *plcl)
{
    fileName = fN;
    pc = plcl;
}

Repeater::~Repeater()
{
}

void Repeater::Prepare() {
    FString file = FPaths::GameDevelopersDir();
    file.Append(fileName);
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
                        actions.Add(LACAction(0, *key, FCString::Atoi(*(*delay)), true));
                    }
                    else {
                        actions.Add(LACAction(0, *key, FCString::Atoi(*(*delay)), false));
                    }
                }
                else if (type->Equals("1")) {
                    //mouse
                    FString* x = new FString(), * y = new FString(), * duration = new FString(), * rest2 = new FString();
                    rest->Split("|", x, rest);
                    rest->Split("|", y, duration);
                    actions.Add(LACAction(1, FCString::Atof(*(*x)), FCString::Atof(*(*y)), FCString::Atof(*(*duration))));
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

void Repeater::Tick() {
    if (actions.Num() == 0) {
        return;
    }
tick_start:
    LACAction* act = &actions[counter];
    if(act->delay == numTicks) {
        if (act->type == 0) {
            //keyboard
            if (act->event == 0) {
                //released
                pc->PlayerInput->InputKey(FKey(TCHAR_TO_ANSI(*act->key)), EInputEvent::IE_Released, 1.0f, false);
            }
            else {
                //pressed
                pc->PlayerInput->InputKey(FKey(TCHAR_TO_ANSI(*act->key)), EInputEvent::IE_Pressed, 1.0f, false);
            }
        }
        else if (act->type == 1) {
            //mouse
            pc->AddYawInput(act->mouseX);
            pc->AddPitchInput(-act->mouseY);
        }
        if (counter < actions.Num() - 1) {
            counter++;
            if (*&actions[counter].delay == numTicks) {
                goto tick_start;
            }
        }
    }
    numTicks++;
}