#include "main_window.h"

#include <string>
#include "renderer.h"
#include "debug.h"

// Window procedure
LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	// WndProc will be used for handling window-related tasks (ie. move, close, resize)
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

		if (MW::GetRenderer()) {
			MW::renderer_->SetDrawArea(&MW::GetDrawRect(), MW::border_width_);
		}
	}
	case WM_SIZE:
	{
		// should this be set and then get? main_rect and draw_rect appear to be 0 initialized at this point
		// Re-establish rect objects for both main window and draw area
		Rect& temp_mwr = MW::GetMainWindowRect();
		MW::SetMainWindowRect(hwnd, &temp_mwr);
		Rect& temp_dr = MW::GetDrawRect();
		MW::SetDrawRect(hwnd, &temp_dr);
		// Re-calculate offsets
		MW::SetMTCOffsetX(((temp_mwr.right - temp_mwr.left) - (temp_dr.right - temp_dr.left)) / 2);
		MW::SetMTCOffsetY(((temp_mwr.bottom - temp_mwr.top) - (temp_dr.bottom - temp_dr.top) - 31) / 2);

		// executed once per program run
		bool first_pass = false;
		if (!MW::GetRenderer()) {
			MW::renderer_ = new Renderer(&temp_dr, MW::border_width_);
			first_pass = true;
		}

		if (MW::GetRenderer() && !first_pass) {
			MW::renderer_->SetDrawArea(&temp_dr, MW::border_width_);
		}
	}
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case (VK_END):
		{
			// allow debug message toggling
			debug::ToggleDebugPrinting();
		} break;
		case (VK_ESCAPE):
		{
			int locked = MW::GetRenderer()->GetFocusLock();
			if (locked != -1) {
				debug::PrintDebugMsg(calling_class::MAIN_WINDOW, debug_type::PANEL_LOCK, &MW::event_message, MW::current_panel_, locked);
				MW::GetRenderer()->SetFocusLock(-1);
				break;
			}
			MW::SetRunningState(STOPPED);
		} break;
		}
	}
	case WM_MOUSEMOVE:
	{
		debug::PrintDebugMsg(calling_class::MAIN_WINDOW, debug_type::MOUSE_POSITION, &MW::event_message, MW::current_panel_);
	} break;
	case WM_LBUTTONDOWN:
	{
		debug::PrintDebugMsg(calling_class::MAIN_WINDOW, debug_type::INPUT_DETECTED, &MW::event_message, MW::current_panel_);

		if (MW::GetRenderer()->GetFocusLock() == -1 && MW::current_panel_ != Renderer::BACKGROUND) {
			MW::GetRenderer()->SetFocusLock(MW::current_panel_);
			debug::PrintDebugMsg(calling_class::MAIN_WINDOW, debug_type::PANEL_LOCK, &MW::event_message, MW::current_panel_);
		}
	} break;
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

	// Calculate offsets between client window and main window
	Rect mwr = MW::GetMainWindowRect();
	Rect dr = MW::GetDrawRect();
	MW::SetMTCOffsetX(((mwr.right - mwr.left) - (dr.right - dr.left)) / 2);
	MW::SetMTCOffsetY(((mwr.bottom - mwr.top) - (dr.bottom - dr.top) - 3) / 2);

	// Clear DrawArea
	MW::GetRenderer()->ClearRenderArea(true);

	// Message loop
	while (MW::GetRunningState()) {
		while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
			// Collect cursor data
			MW::event_message = msg;

			MW::ConditionMouseCoords(MW::event_message.pt);
			MW::current_panel_ = MW::GetCursorFocus((Point)MW::event_message.pt);
			if (MW::current_panel_ != MW::GetRenderer()->GetFocus())
				MW::GetRenderer()->SetFocus(MW::current_panel_);
			
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		MW::GetRenderer()->ClearRenderArea();
		// Draw updated RenderArea to screen
		MW::GetRenderer()->DrawRenderArea(hdc);
	}

	CleanUp();
	return 0;
}

Renderer* main_window::GetRenderer() {
	return renderer_;
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

int main_window::GetCursorFocus(Point p) {

	for (int i = 0; i < Renderer::NUM_PANELS; i++) {
		if (MW::GetRenderer()->draw_area_.aabb[i].Collision(p)) {
			return i;
		}
	}
	return -1;
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
	MW::main_to_client_offset_x_ = p_offset_x;
}

uint8_t main_window::GetMTCOffsetX() {
	return MW::main_to_client_offset_x_;
}

void main_window::SetMTCOffsetY(uint8_t p_offset_y) {
	MW::main_to_client_offset_y_ = p_offset_y;
}

uint8_t main_window::GetMTCOffsetY() {
	return MW::main_to_client_offset_y_;
}

Rect& main_window::GetMainWindowRect() {
	return main_rect;
}

void main_window::SetMainWindowRect(HWND hwnd, Rect* rect) {

	RECT mwr;
	GetWindowRect(hwnd, &mwr);
	MW::main_rect = mwr;
	/*MW::main_rect.left = temp_mwr.left;
	MW::main_rect.top = temp_mwr.top;
	MW::main_rect.right = temp_mwr.right;
	MW::main_rect.bottom = temp_mwr.bottom;*/
}

Rect& main_window::GetDrawRect() {
	
	return draw_rect;
}

void main_window::SetDrawRect(HWND hwnd, Rect* rect) {

	RECT dr;
	GetClientRect(hwnd, &dr);
	MW::draw_rect = dr;
	/*MW::draw_rect.left = temp_dr.left;
	MW::draw_rect.top = temp_dr.top;
	MW::draw_rect.right = temp_dr.right;
	MW::draw_rect.bottom = temp_dr.bottom;*/
}

void main_window::ConditionMouseCoords(Point& p) {
	// likely needs to be updated to include panel detection

	// condition mouse cursor coordinates to be within range
	// 30 pixels is fixed for Y dimension due to title bar
	p.y -= (MW::GetMainWindowRect().top + 31);
	p.x -= (MW::GetMainWindowRect().left + MW::GetMTCOffsetX());
}

void main_window::ConditionMouseCoords(POINT& p) {
	// likely needs to be updated to include panel detection

	// condition mouse cursor coordinates to be within range
	// 30 pixels is fixed for Y dimension due to title bar
	p.y -= (MW::GetMainWindowRect().top + 31);
	p.x -= (MW::GetMainWindowRect().left + MW::GetMTCOffsetX());
}