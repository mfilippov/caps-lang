#include <tchar.h>
#include <windows.h>

#define EXIT_ID 33
#define REHOOK_INTERVAL_MS 30000

static HHOOK kHook;
static UINT_PTR rehookTimerId;

static HHOOK InstallHook(void);

static LRESULT CALLBACK KbdHook(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		KBDLLHOOKSTRUCT *ks = (KBDLLHOOKSTRUCT *)lParam;

		if (!(ks->flags & LLKHF_INJECTED) &&
		    ks->vkCode == VK_CAPITAL &&
		    !(GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
			HWND hWnd = GetForegroundWindow();
			if (hWnd) {
				if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
					PostMessage(hWnd, WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM)HKL_NEXT);
				}
				return 1;
			}
		}
	}
	return CallNextHookEx(kHook, nCode, wParam, lParam);
}

static HHOOK InstallHook(void) {
	return SetWindowsHookEx(WH_KEYBOARD_LL, KbdHook, GetModuleHandle(NULL), 0);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_QUERYENDSESSION) {
		return TRUE;
	}
	if (msg == WM_ENDSESSION && wParam) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

static void failed(const TCHAR *msg) {
	MessageBox(NULL, msg, _T("CapsLang - Error"), MB_OK | MB_ICONERROR);
	ExitProcess(1);
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR cmd, int show) {
	MSG msg;
	BOOL bRet;
	WNDCLASS wc = {0};
	HWND hWndHidden;

	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, _T("CapsLang"));
	if (hEvent == NULL) {
		failed(_T("CreateEvent()"));
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(hEvent);
		failed(_T("CapsLang is already running!"));
		return 0;
	}

	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInst;
	wc.lpszClassName = _T("CapsLangClass");
	RegisterClass(&wc);

	hWndHidden = CreateWindow(wc.lpszClassName, _T("CapsLang"), 0,
		0, 0, 0, 0, HWND_MESSAGE, NULL, hInst, NULL);
	if (!hWndHidden) {
		failed(_T("CreateWindow()"));
	}

	if (RegisterHotKey(hWndHidden, EXIT_ID, MOD_CONTROL | MOD_SHIFT, 'L') == 0) {
		failed(_T("RegisterHotKey()"));
	}

	kHook = InstallHook();
	if (!kHook) {
		failed(_T("SetWindowsHookEx()"));
	}

	rehookTimerId = SetTimer(hWndHidden, 0, REHOOK_INTERVAL_MS, NULL);

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) {
			break;
		}

		if (msg.message == WM_HOTKEY && msg.wParam == EXIT_ID) {
			PostQuitMessage(0);
			continue;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	KillTimer(hWndHidden, rehookTimerId);
	UnhookWindowsHookEx(kHook);
	DestroyWindow(hWndHidden);
	CloseHandle(hEvent);

	return 0;
}
