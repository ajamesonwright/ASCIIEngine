#include "main_window.h"

#include <string>
#include "renderer.h"
#include "debug.h"

// Window procedure
LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	switch (msg)
	{
	case WM_CLOSE:
	case WM_DESTROY:
	{
		MW::SetRunningState(STOPPED);
	} break;
	case WM_MOVE:
	{
		// Re-establish rect objects for both main window and draw area
		GetWindowRect(hwnd, &MW::GetMainWindowRect());
		GetClientRect(hwnd, &MW::GetDrawRect());
		MW::SetMTCOffsetX(((MW::GetMainWindowRect().right - MW::GetMainWindowRect().left) - (MW::GetDrawRect().right - MW::GetDrawRect().left)) / 2);

		if (MW::GetRenderer())
			MW::renderer->SetDrawArea(&MW::GetDrawRect());
	}
	case WM_SIZE:
	{
		RECT rect;
		GetClientRect(hwnd, &rect);

		if (!MW::GetRenderer())
			MW::renderer = new Renderer(&rect);

		if (MW::GetRenderer())
			MW::renderer->SetDrawArea(&rect);
	} break;
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case (VK_END):
			// allow debug message toggling
			MW::print_debug_ = !MW::print_debug_;
		}
	}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

static void CleanUp() {
	MW::GetRenderer()->CleanUp();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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
	hwnd = CreateWindow(wc.lpszClassName, L"ASCII Engine", WS_OVERLAPPEDWINDOW | WS_VISIBLE, MW::window_starting_x_, MW::window_starting_y_, MW::window_starting_width_, MW::window_starting_height_, 0, 0, hInstance, 0);

	if (hwnd == NULL) {
		MessageBox(NULL, L"Window creation unsuccessful", L"Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// Display newly created window
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	// Get device context handle for later use
	HDC hdc = GetDC(hwnd);
	// Establish rendering state
	MW::SetRunningState(RUNNING);
	// Find bounds of main window
	GetWindowRect(hwnd, &MW::GetMainWindowRect());
	// Find bounds of client window within main window
	GetClientRect(hwnd, &MW::GetDrawRect());
	// Acquire renderer instance and apply draw rect dimensions
	//*MW::renderer = Renderer(MW::GetDrawRect());
	//MW::GetRenderer().SetDrawArea(MW::GetDrawRect());
	// Calculate offsets between client window and main window
	MW::SetMTCOffsetX(((MW::GetMainWindowRect().right - MW::GetMainWindowRect().left) - (MW::GetDrawRect().right - MW::GetDrawRect().left)) / 2);
	//MW::main_to_client_offset_y = ((MW::main_rect.bottom - MW::main_rect.top) - (MW::draw_rect.right - MW::draw_rect.left) - 30) / 2;
	
	// Message loop
	int counter = 0;
	while (MW::GetRunningState()) {
		while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
			switch (msg.message)
			{
			case WM_MOUSEMOVE:
			{
				POINT p;
				// If cursor is moving (only valid if cursor is within window focus)
				if (GetCursorPos(&p)) {

					MW::ConditionMouse(p);
					if (MW::print_debug_) {
						debug::PrintDebug(calling_class::MAIN_WINDOW, debug_type::MOUSE_POSITION, p, counter++);
					}

					MW::GetRenderer()->UpdateRenderArea(p);
				}
			} break;
			case WM_MBUTTONDOWN:
			{
				POINT p;
				if (GetCursorPos(&p)) {
					MW::ConditionMouse(p);

					if (MW::print_debug_)
						debug::PrintDebug(calling_class::MAIN_WINDOW, debug_type::MOUSE_MEMORY_LOCATION, p, counter, MW::GetRenderer()->GetMemoryLocation(p));
				}
			} break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		MW::GetRenderer()->DrawRenderArea(hdc);
	}

	CleanUp();
	return 0;
}

Renderer* main_window::GetRenderer() {
	return renderer;
}

bool main_window::GetRunningState() {
	return MW::run_state_;
}

void main_window::SetRunningState(int p_run_state) {
	if (p_run_state == (int) MW::GetRunningState())
	{
		return;
	}
	if (p_run_state != 0 && p_run_state != 1) {
		return;
	}
	MW::run_state_ = p_run_state;
}

void main_window::SetWindowHeight(uint16_t p_height) {
	MW::window_height_ = p_height;
}

uint16_t main_window::GetWindowHeight() {
	return MW::window_height_;
}

void main_window::SetWindowWidth(uint16_t p_width) {
	MW::window_width_ = p_width;
}

uint16_t main_window::GetWindowWidth() {
	return MW::window_width_;
}

void main_window::SetWindowOffsetX(uint16_t p_offset) {
	MW::xPos_ = p_offset;
}

uint16_t main_window::GetWindowOffsetX() {
	return MW::xPos_;
}

void main_window::SetWindowOffsetY(uint16_t p_offset) {
	yPos_ = p_offset;
}

uint16_t main_window::GetWindowOffsetY() {
	return MW::yPos_;
}

void main_window::SetMTCOffsetX(uint8_t p_offset_x) {
	MW::main_to_client_offset_x = p_offset_x;
}

uint8_t main_window::GetMTCOffsetX() {
	return MW::main_to_client_offset_x;
}

void main_window::SetMTCOffsetY(uint8_t p_offset_y) {
	MW::main_to_client_offset_y = p_offset_y;
}

uint8_t main_window::GetMTCOffsetY() {
	return MW::main_to_client_offset_y;
}

RECT& main_window::GetMainWindowRect() {
	return main_rect;
}

RECT& main_window::GetDrawRect() {
	return draw_rect;
}

void main_window::ConditionMouse(POINT& p) {

	// condition mouse cursor coordinates to be within range
	// 30 pixels is fixed for Y dimension
	p.y -= (MW::GetMainWindowRect().top + 30);
	p.x -= (MW::GetMainWindowRect().left + MW::GetMTCOffsetX());
}