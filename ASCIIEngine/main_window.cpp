#include "main_window.h"

#include <string>
#include "renderer.h"
#include "debug.h"

// Window procedure
LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	// WndProc will be used for handling window-related tasks (ie. move, close, resize) and one-time key press events (ESC, end)
	// (toggle debug printing, escape to clear focus, etc).
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
		MW::GetRenderer()->ClearRenderArea(true);
	} break;
	case WM_SIZE:
	{
		// Re-establish rect objects for both main window and draw area
		Rect& temp_mwr = MW::GetMainWindowRect();
		MW::SetMainWindowRect(hwnd, &temp_mwr);
		Rect& temp_dr = MW::GetDrawRect();
		MW::SetDrawRect(hwnd, &temp_dr);
		// Re-calculate offsets
		MW::SetMTCOffsetX((temp_mwr.GetWidth()) - (temp_dr.GetWidth()) / 2);
		MW::SetMTCOffsetY((temp_mwr.GetHeight() - (temp_dr.GetHeight()) - 31) / 2);

		// executed once per program run
		bool first_pass = false;
		if (!MW::GetRenderer()) {
			MW::renderer_ = new Renderer(&temp_dr, MW::border_width_);
			first_pass = true;
		}

		if (MW::GetRenderer() && !first_pass) {
			MW::renderer_->SetDrawArea(&temp_dr, MW::border_width_);
		}
		MW::GetRenderer()->ClearRenderArea(true);
	} break;
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case (VK_END):
		{
			// allow debug messaging toggle
			debug::ToggleDebugPrinting();
		} break;
		case (VK_ESCAPE):
		{
			MW::input_->ClearInput();
			// clear focus lock or exit if no lock present
			int locked = MW::GetRenderer()->GetFocusLock();
			if (locked != -1) {
				debug::PrintDebugMsg(calling_class::MAIN_WINDOW_CLASS, debug_type::PANEL_LOCK, &MW::event_message, MW::current_panel_, locked);
				MW::GetRenderer()->SetFocusLock(-1);
				MW::GetRenderer()->UpdateRenderArea(MW::GetRenderer()->GetDrawArea()->panels[locked], locked, MW::GetRenderer()->colours[locked][MW::current_panel_ == locked ? 1 : 0], true);
				break;
			}
			MW::SetRunningState(STOPPED);
		} break;
		}
	} break;
	case WM_MOUSEMOVE:
	{
		debug::PrintDebugMsg(calling_class::MAIN_WINDOW_CLASS, debug_type::MOUSE_POSITION, &MW::event_message, MW::current_panel_);

		// Verify that the cursor has changed panels since last update and no panel is currently locked
		if (MW::current_panel_ == MW::renderer_->GetFocus() || MW::renderer_->GetFocusLock() != -1)
			break;
		MW::renderer_->SetFocus(MW::current_panel_);

		// Reset TOP_DOWN panel
		if (MW::current_panel_ != Renderer::TOP_DOWN) {
			MW::renderer_->UpdateRenderArea(*MW::GetDrawAreaPanel(Renderer::TOP_DOWN), Renderer::TOP_DOWN, MW::renderer_->colours[Renderer::TOP_DOWN][0], true);
		}
		// Reset FIRST_PERSON panel
		if (MW::current_panel_ != Renderer::FIRST_PERSON) {
			MW::renderer_->UpdateRenderArea(*MW::GetDrawAreaPanel(Renderer::FIRST_PERSON), Renderer::FIRST_PERSON, MW::renderer_->colours[Renderer::FIRST_PERSON][0], true);
		}
		if (MW::current_panel_ == Renderer::BACKGROUND)
			break;

		MW::renderer_->UpdateRenderArea(MW::renderer_->GetDrawArea()->panels[MW::current_panel_], MW::current_panel_, MW::renderer_->colours[MW::current_panel_][1], true);
	} break;
	case WM_LBUTTONDOWN:
	{
		debug::PrintDebugMsg(calling_class::MAIN_WINDOW_CLASS, debug_type::INPUT_DETECTED, &MW::event_message, MW::current_panel_);

		if (MW::GetRenderer()->GetFocusLock() == -1 && MW::current_panel_ != Renderer::BACKGROUND) {
			MW::GetRenderer()->SetFocusLock(MW::current_panel_);
			debug::PrintDebugMsg(calling_class::MAIN_WINDOW_CLASS, debug_type::PANEL_LOCK, &MW::event_message, MW::current_panel_);
			break;
		}

		if (MW::GetRenderer()->GetFocusLock() != -1 && MW::current_panel_ == Renderer::TOP_DOWN) {
			MW::geo_start = MW::event_message.pt;
			MW::input_->input_state[ML_DOWN].held = true;
			MW::input_->input_state[ML_DOWN].update = true;
		}
	} break;
	case WM_LBUTTONUP:
	{
		debug::PrintDebugMsg(calling_class::MAIN_WINDOW_CLASS, debug_type::INPUT_DETECTED, &MW::event_message, MW::current_panel_);

		if (MW::input_->GetInput(ML_DOWN)) {
			// Clear held for each button up message if it qualified as 'click and drag' action
			MW::input_->input_state[ML_DOWN].held = false;
			MW::input_->input_state[ML_DOWN].update = true;
		} else
			break;

		// Check for top-down focus lock before calculating square end point
		if (MW::GetRenderer()->GetFocusLock() == 0 && !MW::input_->input_state[ML_DOWN].held) {
			MW::geo_end = MW::event_message.pt;

			if (MW::geo_start.Displacement(MW::geo_end) > 10) {
				Rect* rect = new Rect(MW::geo_start, MW::geo_end);

				debug::PrintDebugMsg(calling_class::MAIN_WINDOW_CLASS, debug_type::GEO_QUEUE_MOD, &MW::event_message, MW::current_panel_, 1, rect);
				MW::geometry_queue.push_back(rect);
			}
		}
		MW::geo_start = Point2d();
		MW::geo_end = Point2d();
	} break;
	case WM_RBUTTONUP:
	{
		if (MW::geometry_queue.size() > 0 && MW::GetRenderer()->GetFocusLock() == Renderer::TOP_DOWN) {
			debug::PrintDebugMsg(calling_class::MAIN_WINDOW_CLASS, debug_type::GEO_QUEUE_MOD, &MW::event_message, MW::current_panel_, 0, MW::geometry_queue.back());
			MW::geometry_queue.pop_back();
			MW::GetRenderer()->GetDrawArea()->update = true;
		}
		MW::GetRenderer()->ClearRenderArea();
	} break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
return 0;
}

static void CleanUp() {
	MW::GetRenderer()->CleanUp();
	delete MW::GetRenderer();
	delete MW::input_;
	for (int i = 0; i < MW::geometry_queue.size(); i++) {
		delete MW::geometry_queue.at(i);
	}
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
	{
		MW::SetMTCOffsetX(((MW::main_rect.GetWidth()) - (MW::draw_rect.GetWidth())) / 2);
		MW::SetMTCOffsetY(((MW::main_rect.GetHeight()) - (MW::draw_rect.GetHeight()) - 31) / 2);
	}

	// Set initial inputs
	MW::input_ = new Input(&MW::camera);
	// Clear DrawArea
	MW::GetRenderer()->ClearRenderArea(true);
	// Set initial viewport location and orientation
	{
		Rect* td_panel = MW::GetDrawAreaPanel(Renderer::TOP_DOWN);
		MW::camera = Ray2d(td_panel->lt.x + td_panel->GetWidth() / 2, td_panel->lt.y + td_panel->GetHeight() / 2, -90);
		MW::camera.SetSize(20);
	}

	// Establish framerate metrics
	float dt = 1.0f / 60.0f;
	LARGE_INTEGER frame_begin_time, frame_end_time;
	QueryPerformanceCounter(&frame_begin_time);
	float frame_update_frequency;
	{
		LARGE_INTEGER performance_frequency_measure;
		QueryPerformanceFrequency(&performance_frequency_measure);
		frame_update_frequency = (float)performance_frequency_measure.QuadPart;
	}

	// Program loop
	while (MW::GetRunningState()) {
		// Reset inputs
		MW::input_->ClearInput(0, 1);
		MW::GetRenderer()->ClearRenderArea();
		// Main message loop
		while (PeekMessage(&MW::event_message, hwnd, 0, 0, PM_REMOVE)) {
			// Update panel under mouseover for highlight
			MW::ConditionMouseCoords(MW::event_message.pt);
			MW::current_panel_ = MW::GetCursorFocus((Point2d)MW::event_message.pt);

			bool is_held = ((MW::event_message.lParam & (1U << 31)) == 0);
			MW::input_->SetInput(&MW::event_message, is_held);

			TranslateMessage(&MW::event_message);
			DispatchMessage(&MW::event_message);
		}

		// Perform actions for updated input_
		MW::input_->HandleInput(&MW::event_message, dt);
		MW::SimulateFrame(dt);

		MW::GetRenderer()->ClearRenderArea(true);
		// Draw highlight line for click and hold
		if ((GetKeyState(VK_LBUTTON) & 0x80) != 0 && MW::GetRenderer()->GetFocusLock() == 0) {
			MW::GetRenderer()->UpdateRenderArea(Line(MW::geo_start, MW::event_message.pt), Renderer::TOP_DOWN, 0xff00, false);
		}
		// Draw geometry queue from front to back
		MW::renderer_->UpdateRenderArea(MW::camera, Renderer::TOP_DOWN);
		for (int i = 0; i < MW::geometry_queue.size(); i++) {
			MW::renderer_->UpdateRenderArea(*MW::geometry_queue[i], Renderer::TOP_DOWN);
		}
		// Draw updated RenderArea to screen
		MW::GetRenderer()->DrawRenderArea(hdc);

		QueryPerformanceCounter(&frame_end_time);
		dt = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart) / frame_update_frequency;
		//debug::PrintDebugMsg(calling_class::MAIN_WINDOW_CLASS, debug_type::FRAMES_PER_SECOND, &MW::event_message, -1, -1, nullptr, -1, dt);
		frame_begin_time = frame_end_time;
	}

	CleanUp();
	return 0;
}

Renderer* main_window::GetRenderer() {
	return renderer_;
}

Rect* main_window::GetDrawAreaPanel(int panel) {
	return &MW::renderer_->GetDrawArea()->panels[panel];
}

bool main_window::GetRunningState() {
	return MW::run_state_;
}

void main_window::SetRunningState(int p_run_state) {
	if (p_run_state == MW::run_state_)
		return;
	if (p_run_state != 0 && p_run_state != 1)
		return;

	MW::run_state_ = p_run_state;
}

void main_window::SetDrawMode(int p_draw_mode) {

	if (p_draw_mode == MW::draw_mode_)
		return;
	if (p_draw_mode < 0 || p_draw_mode >= D_DRAW_MODE_SIZE)
		return;

	debug::PrintDebugMsg(calling_class::MAIN_WINDOW_CLASS, debug_type::DRAW_MODE_CHANGED, &MW::event_message, -1, -1, nullptr, p_draw_mode);
	MW::draw_mode_ = p_draw_mode;
}

int main_window::GetCursorFocus(Point2d p) {

	for (int i = 0; i < Renderer::NUM_PANELS; i++) {
		if (MW::GetRenderer()->GetDrawArea()->panels[i].Collision(p)) {
			return i;
		}
	}
	return -1;
}

void main_window::SimulateFrame(float dt) {

	// Damping effect on acceleration
	MW::camera.ax = MW::camera.ax * 0.70f;
	MW::camera.ay = MW::camera.ay * 0.70f;
	MW::camera.ClampAcceleration();
	// p = p + v*t + 1/2*a*t^2
	MW::camera.px = MW::camera.px + MW::camera.vx * dt + MW::camera.ax * dt * dt * 0.5f;
	MW::camera.py = MW::camera.py + MW::camera.vy * dt + MW::camera.ay * dt * dt * 0.5f;
	MW::camera.x = (uint32_t)MW::camera.px;
	MW::camera.y = (uint32_t)MW::camera.py;
	MW::camera.ClampPosition(*MW::GetDrawAreaPanel(Renderer::TOP_DOWN));
	// v = v + a*t
	MW::camera.vx = (MW::camera.vx + MW::camera.ax * dt) * 0.90f;
	MW::camera.vy = (MW::camera.vy + MW::camera.ay * dt) * 0.90f;
	MW::camera.ClampVelocity();

	debug::PrintDebugMsg(calling_class::MAIN_WINDOW_CLASS, debug_type::CAMERA_STATUS, &MW::event_message, -1, -1, nullptr, -1, -1.0f, nullptr, &MW::camera);
}

// Find the appropriate memory address that reflects the lower left point of the geometry object
void* main_window::FindMemoryHandle(Geometry* g) {

	return MW::GetRenderer()->GetMemoryLocation(0, *g->vertices.at(0));
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
}

Rect& main_window::GetDrawRect() {
	
	return draw_rect;
}

void main_window::SetDrawRect(HWND hwnd, Rect* rect) {

	RECT dr;
	GetClientRect(hwnd, &dr);
	MW::draw_rect = dr;
}

void main_window::ConditionMouseCoords(Point2d& p) {

	// condition mouse cursor coordinates to be within range
	// 31 pixels is fixed for Y dimension due to title bar
	p.y -= (MW::GetMainWindowRect().lt.y + 31);
	p.x -= (MW::GetMainWindowRect().lt.x + MW::GetMTCOffsetX());
}

void main_window::ConditionMouseCoords(POINT& p) {

	// condition mouse cursor coordinates to be within range
	// 30 pixels is fixed for Y dimension due to title bar
	p.y -= (MW::GetMainWindowRect().lt.y + 31);
	p.x -= (MW::GetMainWindowRect().lt.x + MW::GetMTCOffsetX());
}
