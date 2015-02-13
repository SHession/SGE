// This is the main DLL file.

#include "stdafx.h"
#include "SGE.h"


using namespace SGEFramework;

Game::Game(){
	puts("Hello");

}

Game::~Game(){
	puts("Goodbye");

}

void Game::Run(HINSTANCE hInstance, int nCmdShow){
	InitWindow(hInstance, nCmdShow);
	InitDevice();
}


HRESULT Game::InitWindow( HINSTANCE hInstance, int nCmdShow ){

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof( WNDCLASSEX );
	wcex.style = CS_OWNDC;
	//wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"DirectXTest";
    wcex.hIconSm = NULL;
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

	hInst = hInstance;
	RECT rc = {0,0,640,480};
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	mainWnd = CreateWindow( L"DirectXTest", L"Direct3D 11 DirectXTest", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
	if( !mainWnd)
		return E_FAIL;

	ShowWindow(mainWnd, nCmdShow);


	return S_OK;
}

HRESULT Game::InitDevice(){

	return S_OK;
}

void Game::Initalize(){


}

void Game::Draw(){


}

void Game::Update(){


}

void Game::LoadContent(){


}