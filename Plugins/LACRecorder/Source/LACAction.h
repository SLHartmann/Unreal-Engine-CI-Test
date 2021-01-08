#pragma once

struct LACAction {
	short type = 0;			//0 keyboard/movement, 1 mouse, 2 delay, 3 multiple keystrokes
	FString key = "";		//key specifier
	double delay = 0.0;		//time before keyboard action AND length of mouse sequence
	float mouseX = 0.0f;	//value that the mouse moved on the x axis
	float mouseY = 0.0f;	//value that the mouse moved on the x axis
	bool event = 0;			//0 released, 1 pressed

	//Constructor for keyboard actions
	LACAction(short, FString, double, bool);
	//Constructor for mouse actions
	LACAction(short, float, float, double);
	//Constructor for delays
	LACAction(short, double);
};