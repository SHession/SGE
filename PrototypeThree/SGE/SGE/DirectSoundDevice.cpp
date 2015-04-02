#include "stdafx.h"
#include "SGDS.h"

using namespace SGDS;

DirectSoundDevice::DirectSoundDevice(){


}

DirectSoundDevice::~DirectSoundDevice(){
	CleanUp();
}

HRESULT DirectSoundDevice::InitializeDevice(HWND hWnd){

	HRESULT result = DirectSoundCreate8(NULL, &lpds,NULL);
	if(FAILED(result)) return result;

	lpds->SetCooperativeLevel(hWnd, DSSCL_NORMAL);
	if(FAILED(result)) return result;

	return S_OK;
}

HRESULT DirectSoundDevice::CleanUp(){
	if(lpds) lpds->Release();
	return S_OK;
}
