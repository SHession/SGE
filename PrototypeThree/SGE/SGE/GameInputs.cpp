#include "stdafx.h"
#include "SGE.h"

using namespace SGE::Input;

GameInputs::GameInputs(){


}

void GameInputs::Update(){
	
	for (std::map<Keys, int>::iterator it = state.begin(); it != state.end(); it++)
	{
		if(it->second == 1)
			it->second = 2;
		else if(it->second == 2)
			it->second = 2;
		else if(it->second == 3)
			it->second = 0;
	}

}

void GameInputs::HandleInput(MSG msg){

	try{
		state.at((Keys)msg.wParam);
	}
	catch(const std::out_of_range& oor){
		state[(Keys)msg.wParam] = 0;
		OutputDebugStringW(L"Mapped\n");
	}

	int keyState = state.at((Keys)msg.wParam);

	if(msg.message == WM_KEYDOWN)
	{
		if(keyState == 0){
			keyState = 1;
		}
		else if(keyState == 1){
			keyState = 2;
		}
		else if(keyState == 2){
			keyState = 2;
		}
	}

	if(msg.message == WM_CHAR){
		if(keyState == 0){
			keyState = 1;
		}
		else if(keyState == 1){
			keyState = 2;
		}
		else if(keyState == 2){
			keyState = 2;
		}
	}

	if(msg.message ==  WM_KEYUP )
	{
		keyState = 3;
	}
		
	state.at((Keys)msg.wParam) = keyState;

}

KeyState GameInputs::GetKeyState(Keys key){
	try{
		return (KeyState)state.at(key);
	}
	catch(const std::out_of_range& oor){
		return NotKey;
	}

}