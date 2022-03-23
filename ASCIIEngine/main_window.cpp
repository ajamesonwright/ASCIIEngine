#include "main_window.h"

#include <string>
#include "renderer.h"

main_window* mw;

// Window procedure
LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	switch (msg)
	{
	case WM_CLOSE:
	case WM_DESTROY:
	{
		mw->SetRunningState(STOPPED);
	} break;
	case WM_MOVE:
	{
		// Re-establish rect objects for both main window and draw area
		GetWindowRect(hwnd, &mw->GetMainWindowRect());
		GetClientRect(hwnd, &mw->GetDrawRect());
		// Re-calculate offset
		mw->SetMTCOffsetX(((mw->GetMainWindowRect().right - mw->GetMainWindowRect().left) - (mw->GetDrawRect().right - mw->GetDrawRect().left)) / 2);

		mw->GetRenderer().SetDrawArea(mw->GetDrawRect());
	}
	case WM_SIZE:
	{
		RECT rect;
		GetClientRect(hwnd, &rect);

		mw->GetRenderer().SetDrawArea(rect);
	} break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

static void CleanUp() {
	mw->GetRenderer().CleanUp();
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	const char caClassName[] = "ASCIIEngine";

	// Generate window class
	size_t size = strlen(caClassName) + 1;
	wchar_t wcaClassName[20];
	size_t outSize;
	mbstowcs_s(&outSize, wcaClassName, 20, caClassName, strlen(caClassName));

	WNDCLASS wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = wcaClassName;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;

	// Register window
	if (!RegisterClass(&wc)) {
		MessageBox(NULL, L"Window registration failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	HWND hwnd;
	MSG msg;
	// Create window
	//AdjustWindowRect(&main_rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, true);
	hwnd = CreateWindow(wc.lpszClassName, L"ASCII Engine", WS_OVERLAPPEDWINDOW | WS_VISIBLE, mw->window_starting_x_, mw->window_starting_y_, mw->window_starting_width_, mw->window_starting_height_, 0, 0, hInstance, 0);

	if (hwnd == NULL) {
		MessageBox(NULL, L"Window creation failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// Display newly created window
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	// Get device context handle for later use
	HDC hdc = GetDC(hwnd);
	// Establish rendering state
	mw->SetRunningState(RUNNING);
	// Find bounds of main window
	GetWindowRect(hwnd, &mw->GetMainWindowRect());
	// Find bounds of client window within main window
	GetClientRect(hwnd, &mw->GetDrawRect());
	// Acquire renderer instance and apply draw rect dimensions
	mw->GetRenderer().SetDrawArea(mw->GetDrawRect());
	// Calculate offsets between client window and main window
	mw->SetMTCOffsetX(((mw->GetMainWindowRect().right - mw->GetMainWindowRect().left) - (mw->GetDrawRect().right - mw->GetDrawRect().left)) / 2);
	//main_window::main_to_client_offset_y = ((main_window::main_rect.bottom - main_window::main_rect.top) - (main_window::draw_rect.right - main_window::draw_rect.left) - 30) / 2;
	
	// Message loop
	int counter = 0;
	while (mw->GetRunningState()) {
		while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
			switch (msg.message)
			{
			case VK_ESCAPE:
				mw->SetRunningState(STOPPED);
			case WM_MOUSEMOVE:
			{
				POINT p;
				// If cursor is moving (only valid if cursor is within window focus)
				if (GetCursorPos(&p))
				{
					// condition mouse cursor coordinates to be within range
					// 30 pixels is fixed for Y dimension
					p.y -= (mw->GetMainWindowRect().top + 30);
					p.x -= (mw->GetMainWindowRect().left + mw->GetMTCOffsetX());

					// common debug information
					if (main_window::print_debug_)
					{
						std::string mp = "main_window:\t" + std::to_string(counter++) + "\t Mouse: ( " + std::to_string(p.x) + ", " + std::to_string(p.y) + " ) \n";
						std::wstring stemp = std::wstring(mp.begin(), mp.end());
						LPCWSTR sw = stemp.c_str();
						OutputDebugString(sw);
					}

					mw->GetRenderer().UpdateRenderArea(p);
				}
			}
			case WM_MBUTTONDOWN:
			{ }
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		mw->GetRenderer().DrawRenderArea(hdc);
	}

	CleanUp();
	return 0;
}

Renderer& main_window::GetRenderer() {
	return *renderer;
}

bool main_window::GetRunningState() {
	return main_window::run_state_;
}

void main_window::SetRunningState(int p_run_state) {
	if (p_run_state == (int) mw->GetRunningState())
	{
		return;
	}
	if (p_run_state != 0 && p_run_state != 1) {
		return;
	}
	main_window::run_state_ = p_run_state;
}

void main_window::SetWindowHeight(UINT height) {
	this->window_height_ = height;
}

UINT main_window::GetWindowHeight() {
	return this->window_height_;
}

void main_window::SetWindowWidth(UINT width) {
	this->window_width_ = width;
}

UINT main_window::GetWindowWidth() {
	return this->window_width_;
}

void main_window::SetWindowOffsetX(UINT offset) {
	xPos_ = offset;
}

UINT main_window::GetWindowOffsetX() {
	return this->xPos_;
}

void main_window::SetWindowOffsetY(UINT offset) {
	yPos_ = offset;
}

UINT main_window::GetWindowOffsetY() {
	return this->yPos_;
}

void main_window::SetMTCOffsetX(UINT offset_x) {
	this->main_to_client_offset_x = offset_x;
}

UINT main_window::GetMTCOffsetX() {
	return this->main_to_client_offset_x;
}

void main_window::SetMTCOffsetY(UINT offset_y) {
	this->main_to_client_offset_y = offset_y;
}

UINT main_window::GetMTCOffsetY() {
	return this->main_to_client_offset_y;
}

RECT& main_window::GetMainWindowRect() {
	return main_rect;
}

RECT& main_window::GetDrawRect() {
	return draw_rect;
}