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
		Rect temp_mwr = MW::GetMainWindowRect();
		MW::SetMainWindowRect(hwnd, &temp_mwr);
		Rect temp_dr = MW::GetDrawRect();
		MW::SetDrawRect(hwnd, &temp_dr);

		if (MW::GetRenderer())
			MW::renderer->SetDrawArea(&MW::GetDrawRect());
	} break;
	case WM_SIZE:
	{
		// Re-establish rect objects for both main window and draw area
		Rect temp_mwr = MW::GetMainWindowRect();
		MW::SetMainWindowRect(hwnd, &temp_mwr);
		Rect temp_dr = MW::GetDrawRect();
		MW::SetDrawRect(hwnd, &temp_dr);
		// Re-calculate offsets
		MW::SetMTCOffsetX(((temp_mwr.right - temp_mwr.left) - (temp_dr.right - temp_dr.left)) / 2);
		MW::SetMTCOffsetY(((temp_mwr.bottom - temp_mwr.top) - (temp_dr.bottom - temp_dr.top) - 31) / 2);

		MW::SetDividerRect(&temp_dr);

		if (!MW::GetRenderer()) {
			MW::renderer = new Renderer(&temp_dr);
		}

		if (MW::GetRenderer())
			MW::renderer->SetDrawArea(&temp_dr);
	} break;
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case (VK_END):
		{
			// allow debug message toggling
			MW::print_debug_ = !MW::print_debug_;
		} break;
		case (VK_ESCAPE):
		{
			MW::SetRunningState(STOPPED);
		} break;
		default:
		{
			Rect draw = MW::GetDrawRect();
			Point centre = { (draw.left + draw.right) / 2, (draw.top + draw.bottom) / 2 };
			int half_size = 10;
			Line line = { Point { centre.x - half_size, centre.y}, Point { centre.x + half_size, centre.y } };
			MW::renderer->UpdateRenderArea(line);
		} break;
		}
	}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

static void CleanUp() {
	MW::GetRenderer()->CleanUp();
	delete MW::GetRenderer();
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
	//MW::SetMainWindowRect(hwnd, &MW::GetMainWindowRect());
	// Find bounds of client window within main window
	//MW::SetDrawRect(hwnd, &MW::GetDrawRect());
	// Calculate offsets between client window and main window
	Rect mwr = MW::GetMainWindowRect();
	Rect dr = MW::GetDrawRect();
	MW::SetDividerRect(&dr);
	MW::SetMTCOffsetX(((mwr.right - mwr.left) - (dr.right - dr.left)) / 2);
	MW::SetMTCOffsetY(((mwr.bottom - mwr.top) - (dr.bottom - dr.top) - 31) / 2);
	
	// Message loop
	int counter = 0;
	while (MW::GetRunningState()) {
		// Clear render area
		MW::GetRenderer()->ClearRenderArea();
		while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
			POINT p;

			switch (msg.message)
			{
			case WM_MOUSEMOVE:
			{
				// If cursor is moving (only valid if cursor is within window focus)
				if (GetCursorPos(&p)) {

					MW::ConditionMouse(p);
					if (MW::print_debug_) {
						debug::PrintDebug(calling_class::MAIN_WINDOW, debug_type::MOUSE_POSITION, p, counter++);
						POINT offset = POINT{ MW::GetMTCOffsetX(), MW::GetMTCOffsetY() };
						debug::PrintDebug(calling_class::MAIN_WINDOW, debug_type::WINDOW_OFFSET, offset, counter);
						MW::GetRenderer()->UpdateRenderArea(Point(p));
					}
				}
			} break;
			case WM_MBUTTONDOWN:
			{
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
		// Draw panel divider
		MW::GetRenderer()->UpdateRenderArea(MW::GetDividerRect(), 0x000000, false);
		// Draw updated RenderArea to screen
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

Rect& main_window::GetMouseFocus() {
	return draw_rect;
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

Rect& main_window::GetMainWindowRect() {
	return main_rect;
}

void main_window::SetMainWindowRect(HWND hwnd, Rect* rect) {

	RECT temp_mwr;
	GetWindowRect(hwnd, &temp_mwr);
	MW::main_rect.left = temp_mwr.left;
	MW::main_rect.top = temp_mwr.top;
	MW::main_rect.right = temp_mwr.right;
	MW::main_rect.bottom = temp_mwr.bottom;
	//GetWindowRect(hwnd, &MW::GetMainWindowRect());
}

Rect& main_window::GetDrawRect() {
	
	return draw_rect;
}

void main_window::SetDrawRect(HWND hwnd, Rect* rect) {

	RECT temp_dr;
	GetClientRect(hwnd, &temp_dr);
	//MW::draw_rect = temp_dr;
	MW::draw_rect.left = temp_dr.left;
	MW::draw_rect.top = temp_dr.top;
	MW::draw_rect.right = temp_dr.right;
	MW::draw_rect.bottom = temp_dr.bottom;
	//GetClientRect(hwnd, &MW::GetDrawRect());
}

Rect& main_window::GetDividerRect() {
	return divider_rect;
}

void main_window::SetDividerRect(Rect* rect) {
	//Rect temp_divider = { (temp_dr.left + temp_dr.right - MW::panel_divider_width_) / 2, temp_dr.top, (temp_dr.left + temp_dr.right + MW::panel_divider_width_) / 2, temp_dr.bottom };
	MW::divider_rect.left = (rect->left + rect->right - MW::panel_divider_width_) / 2;
	MW::divider_rect.top = rect->top;
	MW::divider_rect.right = (rect->left + rect->right + MW::panel_divider_width_) / 2;
	MW::divider_rect.bottom = rect->bottom;
}

void main_window::ConditionMouse(POINT& p) {

	// condition mouse cursor coordinates to be within range
	// 30 pixels is fixed for Y dimension
	p.y -= (MW::GetMainWindowRect().top + 30);
	p.x -= (MW::GetMainWindowRect().left + MW::GetMTCOffsetX());
}