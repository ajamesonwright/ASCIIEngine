#include "MainWindow.h"

#include <string>
#include "Renderer.h"
#include "Debug.h"

// Window procedure
LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	// WndProc will be used for handling window-related tasks (ie. move, close, resize) and one-time key press events (ESC, end)
	// (toggle debug printing, escape to clear focus, etc).
	switch (msg)
	{
	case WM_CLOSE:
	case WM_DESTROY:
	{
		MW::setRunningState(STOPPED);
	} break;
	case WM_MOVE:
	{
		// Re-establish rect objects for both main window and draw area
		Rect temp_mwr = MW::GetMainWindowRect();
		MW::setMainWindowRect(hwnd, &temp_mwr);
		Rect temp_dr = MW::GetDrawRect();
		MW::setDrawRect(hwnd, &temp_dr);

		if (MW::GetRenderer()) {
			MW::renderer_->setDrawArea(&MW::GetDrawRect(), MW::border_width_);
		}
		MW::GetRenderer()->ClearRenderArea(true);
	} break;
	case WM_SIZE:
	{
		// Re-establish rect objects for both main window and draw area
		Rect& temp_mwr = MW::GetMainWindowRect();
		MW::setMainWindowRect(hwnd, &temp_mwr);
		Rect& temp_dr = MW::GetDrawRect();
		MW::setDrawRect(hwnd, &temp_dr);
		// Re-calculate offsets
		MW::setMTCOffsetX((temp_mwr.GetWidth()) - (temp_dr.GetWidth()) / 2);
		MW::setMTCOffsetY((temp_mwr.GetHeight() - (temp_dr.GetHeight()) - 31) / 2);

		// executed once per program run
		bool first_pass = false;
		if (!MW::GetRenderer()) {
			MW::renderer_ = new Renderer(&temp_dr, MW::border_width_);
			first_pass = true;
		}

		if (MW::GetRenderer() && !first_pass) {
			MW::renderer_->setDrawArea(&temp_dr, MW::border_width_);
		}
		MW::GetRenderer()->ClearRenderArea(true);
	} break;
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case (VK_END):
		{
			// allow debug messaging toggle
			Debug::ToggleDebugPrinting();
		} break;
		case (VK_ESCAPE):
		{
			MW::input_->clearInput();
			// clear focus lock or exit if no lock present
			int locked = MW::GetRenderer()->GetFocusLock();
			if (locked != -1) {
				Debug::DebugMessage dbg(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::PANEL_LOCK);
				dbg.setMsg(&MW::event_message);
				dbg.setPanelId(MW::current_panel_);
				dbg.setLockedPanel(locked);
				Debug::Print(&dbg);

				MW::GetRenderer()->setFocusLock(-1);
				MW::GetRenderer()->UpdateRenderArea(MW::GetRenderer()->GetDrawArea()->panels[locked], locked, MW::GetRenderer()->colours[locked][MW::current_panel_ == locked ? 1 : 0], true);
				break;
			}
			MW::setRunningState(STOPPED);
		} break;
		}
	} break;
	case WM_MOUSEMOVE:
	{
		Debug::DebugMessage dbg(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::MOUSE_POSITION);
		dbg.setMsg(&MW::event_message);
		dbg.setPanelId(MW::current_panel_);
		dbg.Print();

		// Verify that the cursor has changed panels since last update and no panel is currently locked
		if (MW::current_panel_ == MW::renderer_->GetFocus() || MW::renderer_->GetFocusLock() != -1)
			break;
		MW::renderer_->setFocus(MW::current_panel_);

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
		{
			Debug::DebugMessage dbg(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::INPUT_DETECTED);
			dbg.setMsg(&MW::event_message);
			dbg.setPanelId(MW::current_panel_);
			dbg.Print();
		}

		if (MW::GetRenderer()->GetFocusLock() == -1 && MW::current_panel_ != Renderer::BACKGROUND) {
			MW::GetRenderer()->setFocusLock(MW::current_panel_);
			Debug::DebugMessage dbg(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::PANEL_LOCK);
			dbg.setMsg(&MW::event_message);
			dbg.setPanelId(MW::current_panel_);
			dbg.Print();
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
		{
			Debug::DebugMessage dbg(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::INPUT_DETECTED);
			dbg.setMsg(&MW::event_message);
			dbg.setPanelId(MW::current_panel_);
			dbg.Print();
		}

		if (MW::input_->getInput(ML_DOWN)) {
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
				Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, GEO_QUEUE_MOD);
				dbg.setMsg(&MW::event_message);
				dbg.setPanelId(MW::current_panel_);
				dbg.setDrawMode(1);
				dbg.setGeometry(rect);
				dbg.Print();

				MW::AddGeometry(rect);
				//Debug::DebugMessage dbg2(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::QUADTREE_TOSTRING);
				//dbg2.setOutputString(MW::qt->ToString());
				//dbg2.Print();
				//MW::geometry_queue.push_back(rect);
				//MW::qt->AddGeometry(rect);
			}
		}
		MW::geo_start = Point2d();
		MW::geo_end = Point2d();
	} break;
	case WM_RBUTTONUP:
	{
		if (MW::geometry_queue.size() > 0 && MW::GetRenderer()->GetFocusLock() == Renderer::TOP_DOWN) {
			Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, GEO_QUEUE_MOD);
			dbg.setMsg(&MW::event_message);
			dbg.setPanelId(MW::current_panel_);
			dbg.setDrawMode(0);
			dbg.setGeometry(MW::geometry_queue.back());
			dbg.Print();
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
	for (int i = MW::geometry_queue.size() - 1; i >= 0; i--) {
		delete MW::geometry_queue.at(i);
	}
	delete MW::qt;
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
	MW::setRunningState(RUNNING);

	// Calculate offsets between client window and main window
	{
		MW::setMTCOffsetX(((MW::main_rect.GetWidth()) - (MW::draw_rect.GetWidth())) / 2);
		MW::setMTCOffsetY(((MW::main_rect.GetHeight()) - (MW::draw_rect.GetHeight()) - 31) / 2);
	}

	// set initial inputs
	MW::input_ = new Input(&MW::camera);
	// Clear DrawArea
	MW::GetRenderer()->ClearRenderArea(true);
	// set initial viewport location and orientation
	{
		Rect* td_panel = MW::GetDrawAreaPanel(Renderer::TOP_DOWN);
		MW::camera = Camera(td_panel->lt.x + td_panel->GetWidth() / 2, td_panel->lt.y + td_panel->GetHeight() / 2, -90);
		MW::camera.setSize(20);
	}

	MW::qt = new Quadtree(MW::GetDrawRect());
	//Point2d* p1 = new Point2d(100, 300);
	//MW::qt->AssignPoint(p1);
	////delete p1;
	//Debug::DebugMessage dbg1(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::QUADTREE_TOSTRING);
	//dbg1.setOutputString(MW::qt->ToString());
	//dbg1.Print();
	////Debug::Print(&dbg1);

	//Point2d* p2 = new Point2d(500, 400);
	//MW::qt->AssignPoint(p2);
	////delete p2;
	//Debug::DebugMessage dbg2(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::QUADTREE_TOSTRING);
	//dbg2.setOutputString(MW::qt->ToString());
	//dbg2.Print();
	////Debug::Print(&dbg2);

	//Circle* c = new Circle(Point2d(100, 200), 30);
	//MW::geometry_queue.push_back(c);
	////delete c;
	//Debug::DebugMessage dbg3(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::QUADTREE_TOSTRING);
	//dbg3.setOutputString(MW::qt->ToString());
	//dbg3.Print();
	////Debug::Print(&dbg3);

	//Rect* r = new Rect(Point2d(450, 100), Point2d(500, 300));
	//MW::geometry_queue.push_back(r);
	////delete r;

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
		MW::input_->clearInput(0, 1);
		MW::GetRenderer()->ClearRenderArea();
		// Main message loop
		while (PeekMessage(&MW::event_message, hwnd, 0, 0, PM_REMOVE)) {
			// Update panel under mouseover for highlight
			MW::ConditionMouseCoords(MW::event_message.pt);
			MW::current_panel_ = MW::GetCursorFocus((Point2d)MW::event_message.pt);

			bool is_held = ((MW::event_message.lParam & (1U << 31)) == 0);
			MW::input_->setInput(&MW::event_message, is_held);

			TranslateMessage(&MW::event_message);
			DispatchMessage(&MW::event_message);
		}

		// Perform actions for updated input_
		MW::input_->handleInput(&MW::event_message, dt);
		MW::SimulateFrame(dt);

		MW::GetRenderer()->ClearRenderArea(true);
		// Draw highlight line for click and hold
		if ((GetKeyState(VK_LBUTTON) & 0x80) != 0 && MW::GetRenderer()->GetFocusLock() == 0) {
			MW::GetRenderer()->UpdateRenderArea(Line(MW::geo_start, MW::event_message.pt), Renderer::TOP_DOWN, 0xff00, false);
		}
		// Draw geometry queue from oldest to newest
		MW::renderer_->UpdateRenderArea(MW::camera, Renderer::TOP_DOWN);
		for (int i = 0; i < MW::geometry_queue.size(); i++) {
			MW::renderer_->UpdateRenderArea(MW::geometry_queue[i], Renderer::TOP_DOWN);
		}
		// Draw quadtree grid
		std::vector<Line*>* grid = MW::qt->GetQuadtreeGrid();
		for (int i = 0; i < grid->size(); i++) {
			MW::GetRenderer()->UpdateRenderArea(grid->at(i), Renderer::TOP_DOWN, 0xff0000, false);
		}
		// Draw updated RenderArea to screen
		MW::GetRenderer()->DrawRenderArea(hdc);

		QueryPerformanceCounter(&frame_end_time);
		dt = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart) / frame_update_frequency;
		Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, FRAMES_PER_SECOND);
		dbg.setMsg(&MW::event_message);
		dbg.setFps(dt);
		Debug::Print(&dbg);
		frame_begin_time = frame_end_time;
	}

	CleanUp();
	return 0;
}

Renderer* MainWindow::GetRenderer() {
	return renderer_;
}

Rect* MainWindow::GetDrawAreaPanel(int panel) {
	return &MW::renderer_->GetDrawArea()->panels[panel];
}

bool MainWindow::GetRunningState() {
	return MW::run_state_;
}

void MainWindow::setRunningState(int p_run_state) {
	if (p_run_state == MW::run_state_)
		return;
	if (p_run_state != 0 && p_run_state != 1)
		return;

	MW::run_state_ = p_run_state;
}

void MainWindow::setDrawMode(int p_draw_mode) {

	if (p_draw_mode == MW::draw_mode_)
		return;
	if (p_draw_mode < 0 || p_draw_mode >= D_DRAW_MODE_SIZE)
		return;

	Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, DRAW_MODE_CHANGED);
	dbg.setMsg(&MW::event_message);
	dbg.setDrawMode(p_draw_mode);
	Debug::Print(&dbg);
	MW::draw_mode_ = p_draw_mode;
}

int MainWindow::GetCursorFocus(Point2d p) {

	for (int i = 0; i < Renderer::NUM_PANELS; i++) {
		if (MW::GetRenderer()->GetDrawArea()->panels[i].Collision(p)) {
			return i;
		}
	}
	return -1;
}

void MainWindow::AddGeometry(Geometry* g) {

	MW::geometry_queue.push_back(g);
	MW::qt->AddGeometry(g);
}

void MainWindow::RemoveGeometry(Geometry* g) {

	MW::geometry_queue.pop_back();
	MW::qt->RemoveGeometry(g);
}

void MainWindow::SimulateFrame(float dt) {

	// Damping effect on acceleration
	MW::camera.ax = MW::camera.ax * 0.70f;
	MW::camera.ay = MW::camera.ay * 0.70f;
	MW::camera.ClampAcceleration();
	// v = v + a*t
	MW::camera.vx = (MW::camera.vx + MW::camera.ax * dt) * 0.90f;
	MW::camera.vy = (MW::camera.vy + MW::camera.ay * dt) * 0.90f;
	MW::camera.ClampVelocity();

	float delta_px = MW::camera.vx * dt + MW::camera.ax * dt * dt * 0.5f;
	float delta_py = MW::camera.vy * dt + MW::camera.ay * dt * dt * 0.5f;

	for (int i = 0; i < MW::geometry_queue.size(); i++) {
		// find closest point on geometry and cast ray (draw line) for visual

	}
	// p = p + v*t + 1/2*a*t^2
	MW::camera.px = MW::camera.px + MW::camera.vx * dt + MW::camera.ax * dt * dt * 0.5f;
	MW::camera.py = MW::camera.py + MW::camera.vy * dt + MW::camera.ay * dt * dt * 0.5f;
	MW::camera.x = (uint32_t)(MW::camera.px + 0.75); // 0.75 added to account for truncation due to cast
	MW::camera.y = (uint32_t)(MW::camera.py + 0.75);
	MW::camera.ClampPosition(*MW::GetDrawAreaPanel(Renderer::TOP_DOWN));
	MW::camera.Update();

	Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, CAMERA_STATUS);
	dbg.setMsg(&MW::event_message);
	dbg.setCamera(&MW::camera);
	dbg.Print();
	//Debug::Print(&dbg);
}

// Find the appropriate memory address that reflects the lower left point of the geometry object
void* MainWindow::FindMemoryHandle(Geometry* g) {

	return MW::GetRenderer()->GetMemoryLocation(0, *g->vertices.at(0));
}

void MainWindow::setWindowHeight(uint16_t p_height) {
	MW::window_height_ = p_height;
}

uint16_t MainWindow::GetWindowHeight() {
	return MW::window_height_;
}

void MainWindow::setWindowWidth(uint16_t p_width) {
	MW::window_width_ = p_width;
}

uint16_t MainWindow::GetWindowWidth() {
	return MW::window_width_;
}

void MainWindow::setWindowOffsetX(uint16_t p_offset) {
	MW::xPos_ = p_offset;
}

uint16_t MainWindow::GetWindowOffsetX() {
	return MW::xPos_;
}

void MainWindow::setWindowOffsetY(uint16_t p_offset) {
	yPos_ = p_offset;
}

uint16_t MainWindow::GetWindowOffsetY() {
	return MW::yPos_;
}

void MainWindow::setMTCOffsetX(uint8_t p_offset_x) {
	MW::main_to_client_offset_x_ = p_offset_x;
}

uint8_t MainWindow::GetMTCOffsetX() {
	return MW::main_to_client_offset_x_;
}

void MainWindow::setMTCOffsetY(uint8_t p_offset_y) {
	MW::main_to_client_offset_y_ = p_offset_y;
}

uint8_t MainWindow::GetMTCOffsetY() {
	return MW::main_to_client_offset_y_;
}

Rect& MainWindow::GetMainWindowRect() {
	return main_rect;
}

void MainWindow::setMainWindowRect(HWND hwnd, Rect* rect) {

	RECT mwr;
	GetWindowRect(hwnd, &mwr);
	MW::main_rect = mwr;
}

Rect& MainWindow::GetDrawRect() {
	
	return draw_rect;
}

void MainWindow::setDrawRect(HWND hwnd, Rect* rect) {

	RECT dr;
	GetClientRect(hwnd, &dr);
	MW::draw_rect = dr;
}

void MainWindow::ConditionMouseCoords(Point2d& p) {

	// condition mouse cursor coordinates to be within range
	// 31 pixels is fixed for Y dimension due to title bar
	p.y -= (MW::GetMainWindowRect().lt.y + 31);
	p.x -= (MW::GetMainWindowRect().lt.x + MW::GetMTCOffsetX());
}

void MainWindow::ConditionMouseCoords(POINT& p) {

	// condition mouse cursor coordinates to be within range
	// 31 pixels is fixed for Y dimension due to title bar
	p.y -= (MW::GetMainWindowRect().lt.y + 31);
	p.x -= (MW::GetMainWindowRect().lt.x + MW::GetMTCOffsetX());
}