

#include "stdafx.h"
#include "SGE.h"

using namespace SGEFramework;

GameTime::GameTime(){
	elapsedTime = clock();
	lastTime = elapsedTime;
	deltaTime = (float)elapsedTime - lastTime;
}

void GameTime::Update(){
	elapsedTime = clock();
	deltaTime = (float)elapsedTime - lastTime;
	lastTime = elapsedTime;
}

float GameTime::GetDeltaTime(){
	return deltaTime/CLOCKS_PER_SEC;
}

float GameTime::GetElapsedTime(){
	return elapsedTime/CLOCKS_PER_SEC;
}