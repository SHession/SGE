#include "stdafx.h"
#include "SGE.h"

using namespace SGEFramework;

GameTime::GameTime(){
	elapsedTime = clock();
	deltaTime = 0;
	lastTime = 0;
}

void GameTime::Update(){
	elapsedTime = clock();
	deltaTime = difftime(elapsedTime, lastTime);
	lastTime = elapsedTime;
}

double GameTime::GetDeltaTime(){
	return ((double)deltaTime/CLOCKS_PER_SEC);
}

double GameTime::GetElapsedTime(){
	return ((double)elapsedTime/CLOCKS_PER_SEC);
}