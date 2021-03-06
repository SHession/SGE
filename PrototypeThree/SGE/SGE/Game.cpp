#include "stdafx.h"
#include "SGE.h"
#include <malloc.h>

using namespace SGE::Framework;

Game::Game(){
	graphics = NULL;
	audio = NULL;
}

Game::~Game(){
	
}

HRESULT Game::Run(GameDescription *gameDescription,HINSTANCE hInstance, int nCmdShow){

	HRESULT result;

	//Initialize the game
	result = InitializeWindow(gameDescription, hInstance, nCmdShow);
	if (FAILED(result)){
		MessageBox( NULL, L"Window Initialize Failed", L"Error", MB_OK );
        return E_FAIL;
	}

	//Check graphic device is assigned
	if(!graphics) {
		MessageBox( NULL, L"No Graphics Device Assigned", L"Error", MB_OK );
        return E_FAIL;
	}
	result = graphics->InitializeDevice(gameDescription,mainWnd);
	if (FAILED(result)){
		MessageBox( NULL, L"Graphics Initialize Failed", L"Error", MB_OK );
        return E_FAIL;
	}

	//Check sound device is assigned
	if(!audio) {
		MessageBox( NULL, L"No Sound Device Assigned", L"Error", MB_OK );
        return E_FAIL;
	}
	result = audio->InitializeDevice(mainWnd);
	if (FAILED(result)){
		MessageBox( NULL, L"Sound Initialize Failed", L"Error", MB_OK );
        return E_FAIL;
	}

	//Initialize game logic
	LoadContent();
	result = graphics->ProcessContent();
	if (FAILED(result)){
		MessageBox( NULL, L"Process content failed", L"Error", MB_OK );
        return E_FAIL;
	}
	Initialize();

	gameLoop = true;

	//Begin game loop
	MSG msg = {0};
	while( WM_QUIT != msg.message && gameLoop)
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
           TranslateMessage( &msg );
           DispatchMessage( &msg );		   

		   if(msg.message == WM_KEYDOWN || msg.message == WM_KEYUP || msg.message == WM_CHAR)
		   {
			   input.HandleInput(msg);
		   }
        }

		//Render the application
		Update(gameTime);
		Draw();		
		gameTime.Update();
		input.Update();
    }
	
	//Clean up the application
	CleanUp();
	if(graphics) graphics->CleanUp();
	if(audio) audio->CleanUp();

	return S_OK;
}

HRESULT Game::InitializeWindow(GameDescription *gameDescription, HINSTANCE hInstance, int nCmdShow){
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof( WNDCLASSEX );
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"SGE";
    wcex.hIconSm = NULL;
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

	hInst = hInstance;

	RECT rc = {0,0,640, 480};

	if(gameDescription){
		rc.bottom = gameDescription->height;
		rc.right = gameDescription->width;
	}

	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	mainWnd = CreateWindow( L"SGE", L"SGETest",  WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX ,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );

	if( !mainWnd)
		return E_FAIL;

	ShowWindow(mainWnd, nCmdShow);


	return S_OK;
}

LRESULT CALLBACK Game::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

void Game::LoadContent(){

}

void Game::Initialize(){

}

void Game::Update(GameTime gameTime){

}

void Game::Draw(){

	
}

void Game::CleanUp(){

}

void Game::Exit(){
	gameLoop = false;
}

void Game::Restart(){
	Initialize();
}