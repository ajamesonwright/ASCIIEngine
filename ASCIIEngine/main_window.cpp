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

		if (MW::GetRenderer()) {
			for (int i = 0; i < Renderer::NUM_PANELS; i++) {
				MW::renderer->SetDrawArea(i, &MW::GetDrawRect(), MW::border_width_);
			}
		}
	} break;
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
			MW::renderer = new Renderer(&temp_dr, MW::border_width_);
			first_pass = true;
		}


		if (MW::GetRenderer() && !first_pass) {
			for (int i = 0; i < Renderer::NUM_PANELS; i++) {
				MW::renderer->SetDrawArea(i, &temp_dr, MW::border_width_);
			}
		}
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
		/*	Rect draw = MW::GetDrawRect();
			Point centre = { (draw.left + draw.right) / 2, (draw.top + draw.bottom) / 2 };
			int half_size = 10;
			Line line = { Point { centre.x - half_size, centre.y}, Point { centre.x + half_size, centre.y } };
			MW::renderer->UpdateRenderArea(line);*/
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

	// Calculate offsets between client window and main window
	Rect mwr = MW::GetMainWindowRect();
	Rect dr = MW::GetDrawRect();
	MW::SetMTCOffsetX(((mwr.right - mwr.left) - (dr.right - dr.left)) / 2);
	MW::SetMTCOffsetY(((mwr.bottom - mwr.top) - (dr.bottom - dr.top) - 3) / 2);
	
	// Clear background DrawArea
	MW::GetRenderer()->ClearRenderArea(Renderer::BACKGROUND, 0x00FF00);

	// Message loop
	while (MW::GetRunningState()) {
		// Local variable to store mouse over location
		int panel = 0;

		// Clear topdown and firstperson draw_area_ panels before re-draw
		MW::GetRenderer()->ClearRenderArea(Renderer::TOP_DOWN, 0xFF0000);
		MW::GetRenderer()->ClearRenderArea(Renderer::FIRST_PERSON, 0xFF);
		while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
			POINT mouse_pos;
			Point p;
			// If cursor is moving (only valid if cursor is within window focus)
			if (GetCursorPos(&mouse_pos)) {
				p = mouse_pos;
				MW::ConditionMouse(p);
				panel = MW::GetMouseFocus(p);
			}

			switch (msg.message)
			{
			case WM_MOUSEMOVE:
			{
				if (MW::print_debug_) {
					// Print counter iteration and mouse position
					debug::PrintDebug(calling_class::MAIN_WINDOW, debug_type::MOUSE_POSITION, p, MW::counter++, nullptr, panel);
					
					// Print mouse over panel_id
					debug::PrintDebug(calling_class::MAIN_WINDOW, debug_type::PANEL_ID, p, 0, nullptr, panel);
					if (panel == 2)
						MW::GetRenderer()->UpdateRenderArea(panel, p);
				}
			} break;
			case WM_MBUTTONDOWN:
			{
				if (MW::print_debug_)
					debug::PrintDebug(calling_class::MAIN_WINDOW, debug_type::MOUSE_MEMORY_LOCATION, p, 0, MW::GetRenderer()->GetMemoryLocation(panel, p));
			} break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Draw updated RenderArea to screen
		for (int i = 0; i < Renderer::NUM_PANELS; i++) {
			if (MW::GetRenderer()->draw_area_[i].update)
				MW::GetRenderer()->DrawRenderArea(i, hdc);
		}
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

int main_window::GetMouseFocus(Point p) {

	Renderer::DrawArea da;
	Rect rect;
	AABB aabb;
	for (int i = 0; i < Renderer::NUM_PANELS; i++) {
		da = MW::GetRenderer()->draw_area_[i];
		rect = Rect(da.xPos, da.yPos - da.height, da.xPos + da.width, da.yPos);
		aabb = AABB(rect);
		if (aabb.Collision(p))
			return i;
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

void main_window::ConditionMouse(Point& p) {
	// likely needs to be updated to include panel detection


	// condition mouse cursor coordinates to be within range
	// 30 pixels is fixed for Y dimension due to title bar
	p.y -= (MW::GetMainWindowRect().top + 30);
	p.x -= (MW::GetMainWindowRect().left + MW::GetMTCOffsetX());
}