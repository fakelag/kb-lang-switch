#include "windows.h"
#include "resource1.h"
#include <iostream>
#include <shellapi.h>
#include <commctrl.h>

#define WND_CLASSNAME "LangSwitch"
#define ID_EXIT 2000

#define VK_A 0x41
#define VK_S 0x53

int jpLangId = MAKELANGID( LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN );
int fiLangId = MAKELANGID( LANG_FINNISH, SUBLANG_FINNISH_FINLAND );

bool isJapan = false;
bool isEnabled = true;

void ChangeKeyboardInputLanguage( int nLanguageId )
{
	HWND hWnd = GetForegroundWindow();

	char szLanguageId[ 9 ];
	snprintf( szLanguageId, sizeof( szLanguageId ), "0000%04x", nLanguageId );

	HKL kbLayout = LoadKeyboardLayoutA( szLanguageId, KLF_ACTIVATE );

	SendMessage( hWnd,
		WM_INPUTLANGCHANGEREQUEST,
		0,
		( LPARAM ) kbLayout
	);

	SendMessage( hWnd,
		WM_INPUTLANGCHANGE,
		0,
		( LPARAM ) kbLayout
	);
}

void RemoveTrayIcon( HWND hWnd, UINT uID )
{
	NOTIFYICONDATA  nid;
	nid.hWnd = hWnd;
	nid.uID = uID;
	Shell_NotifyIcon( NIM_DELETE, &nid );
}

void MakeTrayIcon( HWND hWnd, UINT uID, UINT uCallbackMsg, UINT uIcon, bool bModifyExisting )
{
	NOTIFYICONDATA  nid;
	nid.hWnd = hWnd;
	nid.uID = uID;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = uCallbackMsg;
	nid.hIcon =
		( HICON ) LoadImage( GetModuleHandle( NULL ),
			MAKEINTRESOURCE( uIcon ),
			IMAGE_ICON,
			GetSystemMetrics( SM_CXSMICON ),
			GetSystemMetrics( SM_CYSMICON ),
			LR_DEFAULTCOLOR );

	Shell_NotifyIcon( bModifyExisting ? NIM_MODIFY : NIM_ADD, &nid );
	DestroyIcon( nid.hIcon );
}

BOOL ShowPopupMenu( HWND hWnd, POINT* curpos, int wDefaultItem )
{
	HMENU hPop = CreatePopupMenu();
	InsertMenu( hPop, 1, MF_BYPOSITION | MF_STRING, ID_EXIT, "Exit" );

	SetMenuDefaultItem( hPop, ID_EXIT, FALSE );
	SetFocus( hWnd );
	SendMessage( hWnd, WM_INITMENUPOPUP, ( WPARAM ) hPop, 0 );

	POINT pt;
	if ( !curpos )
	{
		GetCursorPos( &pt );
		curpos = &pt;
	}

	WORD cmd = TrackPopupMenu( hPop, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, curpos->x, curpos->y, 0, hWnd, NULL );
	SendMessage( hWnd, WM_COMMAND, cmd, 0 );
	DestroyMenu( hPop );
	return 0;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
	case WM_CREATE:
		ChangeKeyboardInputLanguage( fiLangId );
		MakeTrayIcon( hWnd, 1, WM_APP, IDI_ICON_ON, false );
		return 0;
	case WM_CLOSE:
		RemoveTrayIcon( hWnd, 1 );
		PostQuitMessage( 0 );
		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case ID_EXIT:
			PostMessage( hWnd, WM_CLOSE, 0, 0 );
			return 0;
		}
		return 0;
	case WM_HOTKEY:
	{
		if ( !isEnabled )
		{
			return 0;
		}

		if ( LOWORD( wParam ) == 1 )
		{
			isJapan = !isJapan;
			ChangeKeyboardInputLanguage( isJapan ? jpLangId : fiLangId );
		}

		if ( LOWORD( wParam ) == 2 )
		{
			INPUT ip;
			ip.type = INPUT_KEYBOARD;
			ip.ki.wScan = 0;
			ip.ki.time = 0;
			ip.ki.dwExtraInfo = 0;
			ip.ki.wVk = VK_KANJI;
			ip.ki.dwFlags = 0;
			SendInput( 1, &ip, sizeof( INPUT ) );
		}

		return 0;
	}
	case WM_APP:
		switch ( lParam )
		{
		case WM_LBUTTONDBLCLK:
			isEnabled = !isEnabled;
			MakeTrayIcon( hWnd, 1, WM_APP, isEnabled ? IDI_ICON_ON : IDI_ICON_OFF, true );
			break;
		case WM_RBUTTONUP:
			ShowPopupMenu( hWnd, NULL, -1 );
		}

		return 0;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE prev, LPSTR cmdline, int show )
{
	HWND hPrev = NULL;

	if ( hPrev = FindWindowA( WND_CLASSNAME, "LangSwitch" ) )
	{
		return 0;
	}

	WNDCLASSEX wclx;
	memset( &wclx, 0, sizeof( wclx ) );

	wclx.cbSize = sizeof( wclx );
	wclx.style = 0;
	wclx.lpfnWndProc = &WndProc;
	wclx.cbClsExtra = 0;
	wclx.cbWndExtra = 0;
	wclx.hInstance = hInst;
	wclx.hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_ICON_MAIN ) );
	wclx.hIconSm = LoadIcon( hInst, MAKEINTRESOURCE( IDI_ICON_MAIN ) );

	wclx.hCursor = LoadCursor( NULL, IDC_ARROW );
	wclx.hbrBackground = ( HBRUSH ) ( COLOR_BTNFACE + 1 );
	wclx.lpszMenuName = NULL;
	wclx.lpszClassName = WND_CLASSNAME;
	RegisterClassEx( &wclx );

	HWND hWnd = CreateWindow( WND_CLASSNAME, "LangSwitch", WS_OVERLAPPEDWINDOW, 100, 100, 250, 150, NULL, NULL, hInst, NULL );

	if ( !hWnd )
	{
		return 1;
	}

	ShowWindow( hWnd, HIDE_WINDOW );

	RegisterHotKey( hWnd, 1, MOD_ALT | MOD_NOREPEAT, VK_A );
	RegisterHotKey( hWnd, 2, MOD_ALT | MOD_NOREPEAT, VK_S );

	MSG msg;
	while ( GetMessage( &msg, NULL, 0, 0 ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	UnregisterClass( WND_CLASSNAME, hInst );
	return 0;
}
