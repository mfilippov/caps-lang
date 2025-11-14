#include <tchar.h>

#include <windows.h>

#define WH_KEYBOARD_LL 13
#define EXIT_ID 33

HHOOK kHook;

LRESULT CALLBACK KbdHook(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode < 0) {
		return CallNextHookEx(kHook, nCode, wParam, lParam);
	}
	if (nCode == HC_ACTION) {
		KBDLLHOOKSTRUCT * ks = (KBDLLHOOKSTRUCT * ) lParam;
		if (ks -> vkCode == VK_CAPITAL && (GetKeyState(VK_SHIFT) >= 0)) {
			if (wParam == WM_KEYDOWN) {
				HWND hWnd = GetForegroundWindow();
				if (hWnd) {
					PostMessage(hWnd, WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM) HKL_NEXT);
					return TRUE;
				}
			}
		}
	}
	return CallNextHookEx(kHook, nCode, wParam, lParam);
}

void failed(const TCHAR * msg) {
	MessageBox(NULL, msg, _T("CapsLang - Error"), MB_OK | MB_ICONERROR);
	ExitProcess(1);
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR cmd, int show) {
	MSG msg;

	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, _T("CapsLang"));
	if (hEvent == NULL) {
		failed(_T("CreateEvent()"));
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		failed(_T("CapsLang is already running!"));
		return 0;
	}

	if (RegisterHotKey(0, EXIT_ID, MOD_CONTROL | MOD_SHIFT, 'L') == 0) {
		failed(_T("RegisterHotKey()"));
	}

	kHook = SetWindowsHookEx(WH_KEYBOARD_LL, KbdHook, GetModuleHandle(0), 0);
	if (kHook == 0) {
		failed(_T("SetWindowsHookEx()"));
	}

	while (GetMessage( & msg, 0, 0, 0)) {
		TranslateMessage( & msg);
		if (msg.message == WM_HOTKEY && msg.wParam == EXIT_ID) {
			PostQuitMessage(0);
		}
		DispatchMessage( & msg);
	}

	UnhookWindowsHookEx(kHook);
	CloseHandle(hEvent);

	ExitProcess(0);
	return 0;
}