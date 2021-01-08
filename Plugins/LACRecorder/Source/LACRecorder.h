#pragma once

class LACRecorder {
public:
	LACRecorder();
	void BeginPlay(APlayerController *);
	void Tick(double);

private:
	void getAllBoundKeys();
	void checkForRecording();
	void recordLACSequence(double);

	TArray<FString> boundKeys;
	TArray<LACAction> sequence;

	APlayerController* pc;

	bool recording = false;
	bool mouseKeyboard = 0;
	//true = movement, false = multiple keystrokes
	bool movement = true;
	float mouseXRec = 0.0f;
	float mouseYRec = 0.0f;
	double numTicks = 0.0f;
};
