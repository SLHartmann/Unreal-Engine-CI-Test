#include "LACAction.h"
#include "Containers/UnrealString.h"

LACAction::LACAction(short t, FString k, double d, bool e) {
	type = t;
	key = k;
	delay = d;
	event = e;
}

LACAction::LACAction(short t, float x, float y, double d) {
	type = t;
	mouseX = x;
	mouseY = y;
	delay = d;
}

LACAction::LACAction(short t, double d) {
	type = t;
	delay = d;
}