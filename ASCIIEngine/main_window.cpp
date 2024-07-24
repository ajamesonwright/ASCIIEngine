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
		MW::setRunningState(STOPPED);
	} break;
	case WM_MOVE:
	{
		// Re-establish rect objects for both main window and draw area
		Rect temp_mwr = MW::getMainWindowRect();
		MW::setMainWindowRect(hwnd, &temp_mwr);
		Rect temp_dr = MW::getDrawRect();
		MW::setDrawRect(hwnd, &temp_dr);

		if (MW::getRenderer()) {
			MW::renderer_->setDrawArea(&MW::getDrawRect(), MW::border_width_);
		}
		MW::getRenderer()->clearRenderArea(true);
	} break;
	case WM_SIZE:
	{
		// Re-establish rect objects for both main window and draw area
		Rect& temp_mwr = MW::getMainWindowRect();
		MW::setMainWindowRect(hwnd, &temp_mwr);
		Rect& temp_dr = MW::getDrawRect();
		MW::setDrawRect(hwnd, &temp_dr);
		// Re-calculate offsets
		MW::setMTCOffsetX((temp_mwr.getWidth()) - (temp_dr.getWidth()) / 2);
		MW::setMTCOffsetY((temp_mwr.getHeight() - (temp_dr.getHeight()) - 31) / 2);

		// executed once per program run
		bool first_pass = false;
		if (!MW::getRenderer()) {
			MW::renderer_ = new Renderer(&temp_dr, MW::border_width_);
			first_pass = true;
		}

		if (MW::getRenderer() && !first_pass) {
			MW::renderer_->setDrawArea(&temp_dr, MW::border_width_);
		}
		MW::getRenderer()->clearRenderArea(true);
	} break;
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case (VK_END):
		{
			// allow debug messaging toggle
			Debug::ToggleDebugPrinting();
		} break;
		case (0x4C):
		{
			// 'L'
			MW::setDrawMode(DrawMode::D_LINE);
		} break;
		case (0x54):
		{
			// 'T'
			MW::setDrawMode(DrawMode::D_TRI);
		} break;
		case (0x52):
		{
			// 'R'
			MW::setDrawMode(DrawMode::D_RECT);
		} break;
		case (0x51):
		{
			// 'Q'
			MW::setDrawMode(DrawMode::D_QUAD);
		} break;
		case (0x43):
		{
			// 'C'
			MW::setDrawMode(DrawMode::D_CIRCLE);
		} break;
		case (VK_ESCAPE):
		{
			MW::input_->clearInput();
			// clear focus lock or exit if no lock present
			int locked = MW::getRenderer()->getFocusLock();
			if (locked != -1) {
				Debug::DebugMessage dbg(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::PANEL_LOCK);
				dbg.setMsg(&MW::event_message);
				dbg.setPanelId(MW::current_panel_);
				dbg.setLockedPanel(locked);
				Debug::Print(&dbg);

				MW::getRenderer()->setFocusLock(-1);
				MW::getRenderer()->updateRenderArea(MW::getRenderer()->getDrawArea()->panels[locked], locked, MW::getRenderer()->colours[locked][MW::current_panel_ == locked ? 1 : 0], true);
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
		if (MW::current_panel_ == MW::renderer_->getFocus() || MW::renderer_->getFocusLock() != -1)
			break;
		MW::renderer_->setFocus(MW::current_panel_);

		// Reset TOP_DOWN panel
		if (MW::current_panel_ != Renderer::TOP_DOWN) {
			MW::renderer_->updateRenderArea(*MW::getDrawAreaPanel(Renderer::TOP_DOWN), Renderer::TOP_DOWN, MW::renderer_->colours[Renderer::TOP_DOWN][0], true);
		}
		// Reset FIRST_PERSON panel
		if (MW::current_panel_ != Renderer::FIRST_PERSON) {
			MW::renderer_->updateRenderArea(*MW::getDrawAreaPanel(Renderer::FIRST_PERSON), Renderer::FIRST_PERSON, MW::renderer_->colours[Renderer::FIRST_PERSON][0], true);
		}
		if (MW::current_panel_ == Renderer::BACKGROUND)
			break;

		MW::renderer_->updateRenderArea(MW::renderer_->getDrawArea()->panels[MW::current_panel_], MW::current_panel_, MW::renderer_->colours[MW::current_panel_][1], true);
	} break;
	case WM_LBUTTONDOWN:
	{
		bool insideExistingGeometry = false;
		Point2d start = MW::event_message.pt;
		for (const auto g : MW::geometryQueue) {
			if (g->checkCollisionWith(start)) {
				// Disallow starting new geometry when starting from insde existing geometry
				insideExistingGeometry = true;
			}
		}
		{
			Debug::DebugMessage dbg(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::INPUT_DETECTED);
			dbg.setMsg(&MW::event_message);
			dbg.setPanelId(MW::current_panel_);
			dbg.Print();
		}

		if (insideExistingGeometry) {
			break;
		}
		if (MW::getRenderer()->getFocusLock() == -1 && MW::current_panel_ != Renderer::BACKGROUND) {
			MW::getRenderer()->setFocusLock(MW::current_panel_);
			Debug::DebugMessage dbg(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::PANEL_LOCK);
			dbg.setMsg(&MW::event_message);
			dbg.setPanelId(MW::current_panel_);
			dbg.Print();
			break;
		}

		if (MW::getRenderer()->getFocusLock() != -1 && MW::current_panel_ == Renderer::TOP_DOWN) {
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
		if (MW::getRenderer()->getFocusLock() == 0 && !MW::input_->input_state[ML_DOWN].held) {
			MW::geo_end = MW::event_message.pt;

			if (MW::geo_start.displacementFrom(MW::geo_end) > 10) {
				Geometry* g;

				Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, GEO_QUEUE_MOD);
				dbg.setMsg(&MW::event_message);
				dbg.setPanelId(MW::current_panel_);
				dbg.setDrawMode(MW::draw_mode_);

				switch (MW::draw_mode_) {
				case (D_TRI): {
					// draw a triangle
					Tri* tri = new Tri(MW::highlightLine.vertices.at(0), MW::highlightLine.vertices.at(1), Point2d(100, 100));
					g = tri;
				} break;
				case (D_QUAD): {
					// draw a quad
					Point2d b = Point2d(MW::highlightLine.vertices.at(0).x, MW::highlightLine.vertices.at(1).y);
					Point2d d = Point2d(MW::highlightLine.vertices.at(1).x, MW::highlightLine.vertices.at(0).y);
					Quad* quad = new Quad(MW::highlightLine.vertices.at(0), b, MW::highlightLine.vertices.at(1), d);
					g = quad;
				} break;
				case (D_CIRCLE): {
					// draw a circle
					Circle* circ = new Circle(MW::highlightLine.vertices.at(0), MW::highlightLine.vertices.at(1));
					g = circ;
				} break;
				default: {
					// draw a rectangle
					Rect* rect = new Rect(MW::highlightLine.vertices.at(0), MW::highlightLine.vertices.at(1));
					g = rect;
				}
				}
				//Rect* rect = new Rect(MW::highlightLine.vertices.at(0), MW::highlightLine.vertices.at(1));

				/*Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, GEO_QUEUE_MOD);
				dbg.setMsg(&MW::event_message);
				dbg.setPanelId(MW::current_panel_);
				dbg.setDrawMode(1);
				dbg.setGeometry(rect);*/
				dbg.setGeometry(g);
				dbg.Print();

				MW::addGeometry(g);
			}
		}
		MW::geo_start = Point2d();
		MW::geo_end = Point2d();
	} break;
	case WM_RBUTTONUP:
	{
		if (MW::geometryQueue.size() > 0 && MW::getRenderer()->getFocusLock() == Renderer::TOP_DOWN) {
			Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, GEO_QUEUE_MOD);
			dbg.setMsg(&MW::event_message);
			dbg.setPanelId(MW::current_panel_);
			dbg.setDrawMode(0);
			dbg.setGeometry(MW::geometryQueue.back());
			dbg.Print();

			MW::removeGeometry(MW::geometryQueue.back());
			MW::getRenderer()->getDrawArea()->update = true;
		}
		MW::getRenderer()->clearRenderArea();
	} break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
return 0;
}

static void cleanUp() {
	MW::getRenderer()->cleanUp();
	delete MW::getRenderer();
	delete MW::input_;
	for (size_t i = 0; i < MW::geometryQueue.size(); i++) {
		delete MW::geometryQueue.at(i);
	}
	delete MW::qt;
	delete MW::camera;
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
		MW::setMTCOffsetX(((MW::main_rect.getWidth()) - (MW::draw_rect.getWidth())) / 2);
		MW::setMTCOffsetY(((MW::main_rect.getHeight()) - (MW::draw_rect.getHeight()) - 31) / 2);
	}

	// Clear DrawArea
	MW::getRenderer()->clearRenderArea(true);
	// set initial viewport location and orientation
	{
		Rect* td_panel = MW::getDrawAreaPanel(Renderer::TOP_DOWN);
		MW::camera = new Camera(td_panel->lt.x + td_panel->getWidth() / 2, td_panel->lt.y + td_panel->getHeight() / 2, -90);
		MW::camera->setSize(20);
	    
	    // set initial inputs
		MW::input_ = new Input(MW::camera);
	}

	// Create base quadtree
	MW::qt = new Quadtree(*MW::getDrawAreaPanel(Renderer::TOP_DOWN));

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
	while (MW::getRunningState()) {
		// Reset inputs
		MW::input_->clearInput(0, 1);
		MW::getRenderer()->clearRenderArea();
		// Main message loop
		while (PeekMessage(&MW::event_message, hwnd, 0, 0, PM_REMOVE)) {
			// Update panel under mouseover for highlight
			MW::conditionMouseCoords(MW::event_message.pt);
			MW::current_panel_ = MW::getCursorFocus((Point2d)MW::event_message.pt);

			bool is_held = ((MW::event_message.lParam & (1U << 31)) == 0);
			MW::input_->setInput(&MW::event_message, is_held);

			TranslateMessage(&MW::event_message);
			DispatchMessage(&MW::event_message);
		}

		// Perform actions for updated input_
		MW::input_->handleInput(&MW::event_message, dt);
		MW::simulateFrame(dt);

		MW::getRenderer()->clearRenderArea(true);
		// Draw highlight line for click and hold
		if (MW::canDrawHightlightLine()) {
			// Cast to custom point type and clamp to window dimensions
			Point2d end = MW::event_message.pt;
			MW::getRenderer()->clampDimension(end.x, Renderer::Dimension::HORIZONTAL, Renderer::BACKGROUND);
			MW::getRenderer()->clampDimension(end.y, Renderer::Dimension::VERTICAL, Renderer::BACKGROUND);
			MW::highlightLine = Line(MW::geo_start, end);

			std::vector<Point2d> collisions;
			std::vector<Line> sides;
			for (Rect rect : MW::getRenderer()->getDrawArea()->panels) {
				MW::highlightLine.checkCollisionWith(rect, collisions, sides);
			}

			std::vector<Line> interferingSides;
			// Check for collisions against current geometry
			for (int i = 0; i < MW::geometryQueue.size(); i++) {
				MW::highlightLine.checkCollisionWith(MW::geometryQueue[i], collisions, interferingSides);
			}
			// Check for collisions against camera
			MW::highlightLine.checkCollisionWith(MW::camera, collisions, sides);

			int colour = 0xff00;
			if (!collisions.empty()) {
				colour = 0xff0000;
				// Sort collisions by displacement from start of highlight line if required
				if (collisions.size() > 1) {
					Point2d start = MW::geo_start;
					std::sort(collisions.begin(), collisions.end(), [&start](Point2d& p1, Point2d& p2) {
						return p1.displacementFrom(start) < p2.displacementFrom(start);
					});
				}
				// Clip to the geometry closest to start of line
				MW::highlightLine = Line(MW::geo_start, Point2d(collisions.at(0)));
			}
			MW::getRenderer()->updateRenderArea(MW::highlightLine, Renderer::TOP_DOWN, colour, false);

			if (MW::outlineType) {
				MW::getRenderer()->updateRenderArea(Line(MW::geo_start, Point2d(MW::geo_start.x, end.y)), Renderer::TOP_DOWN, 0xff00, false);
				MW::getRenderer()->updateRenderArea(Line(MW::geo_start, Point2d(end.x, MW::geo_start.y)), Renderer::TOP_DOWN, 0xff00, false);

				MW::getRenderer()->updateRenderArea(Line(Point2d(MW::geo_start.x, end.y), end), Renderer::TOP_DOWN, 0xff00, false);
				MW::getRenderer()->updateRenderArea(Line(Point2d(end.x, MW::geo_start.y), end), Renderer::TOP_DOWN, 0xff00, false);
			}
		}
		// Draw geometry queue from oldest to newest
		MW::renderer_->updateRenderArea(*MW::camera, Renderer::TOP_DOWN, MW::camera->colour);
		for (int i = 0; i < MW::geometryQueue.size(); i++) {
			MW::renderer_->updateRenderArea(MW::geometryQueue[i], Renderer::TOP_DOWN);
		}
		// Draw quadtree grid
		std::vector<Line*>* grid = MW::qt->getQuadtreeGrid();
		for (int i = 0; i < grid->size(); i++) {
			MW::getRenderer()->updateRenderArea(grid->at(i), Renderer::TOP_DOWN, 0xff0000, true);
		}
		// Draw updated RenderArea to screen
		MW::getRenderer()->drawRenderArea(hdc);

		QueryPerformanceCounter(&frame_end_time);
		dt = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart) / frame_update_frequency;
		Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, FRAMES_PER_SECOND);
		dbg.setMsg(&MW::event_message);
		dbg.setFps(dt);
		Debug::Print(&dbg);
		frame_begin_time = frame_end_time;
	}

	cleanUp();
	return 0;
}

Renderer* MainWindow::getRenderer() {
	return renderer_;
}

Rect* MainWindow::getDrawAreaPanel(int panel) {
	return &MW::renderer_->getDrawArea()->panels[panel];
}

bool MainWindow::getRunningState() {
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

int MainWindow::getCursorFocus(Point2d p) {

	for (int i = 0; i < Renderer::NUM_PANELS; i++) {
		if (MW::getRenderer()->getDrawArea()->panels[i].checkCollisionWith(p)) {
			return i;
		}
	}
	return -1;
}

void MainWindow::addGeometry(Geometry* g) {

	// We will want to clip the geometry before adding it to the queue, otherwise we have to clip it every frame
	MW::geometryQueue.push_back(g);
	MW::qt->addGeometry(g);
}

void MainWindow::removeGeometry(Geometry* g) {

	MW::geometryQueue.pop_back();
	MW::qt->removeGeometry(g);
}

void MainWindow::simulateFrame(float dt) {

	// Damping effect on acceleration
	MW::camera->ax = MW::camera->ax * 0.70f;
	MW::camera->ay = MW::camera->ay * 0.70f;
	MW::camera->clampAcceleration();
	// v = v + a*t
	MW::camera->vx = (MW::camera->vx + MW::camera->ax * dt) * 0.90f;
	MW::camera->vy = (MW::camera->vy + MW::camera->ay * dt) * 0.90f;
	MW::camera->clampVelocity();
	// Incremental change in position
	float dPx = MW::camera->vx * dt + MW::camera->ax * dt * dt * 0.5f;
	float dPy = MW::camera->vy * dt + MW::camera->ay * dt * dt * 0.5f;

	// Angular acceleration
	MW::camera->aa = MW::camera->aa * 0.70f;
	MW::camera->clampAngularAcceleration();
	// Angular velocity
	MW::camera->va = (MW::camera->va + MW::camera->aa * dt) * 0.90f;
	// Incremental change in direction
	float dTheta = MW::camera->va * dt;

	bool collision = false;
	Line xAxis = Line(Point2d(0, 0), Point2d(1, 0));
	for (int i = 0; i < MW::geometryQueue.size(); i++) {
		std::vector<Point2d> collisions;
		std::vector<Line> interferingSides;
		MW::camera->leftSide.checkCollisionWith(MW::geometryQueue[i], collisions, interferingSides);
		MW::camera->rightSide.checkCollisionWith(MW::geometryQueue[i], collisions, interferingSides);

		if (!collisions.empty()) {
			collision = true;
			// Determine normal of each collision and adjust position of camera to remove collision
			for (Point2d c : collisions) {
				Line side = interferingSides.at(0);
				//MW::camera->clampPosition(c, side);
				//Point2d closest = MW::camera->findClosestBoundingVertex(c);
				float angleOfInterferingSideToXAxis = xAxis.calculateAngle(interferingSides.at(0));
				float cosx = cos(angleOfInterferingSideToXAxis);
				float siny = sin(angleOfInterferingSideToXAxis);

				if ((c.x - MW::camera->x) * MW::camera->ax > 0) {
					dPx *= cosx;
				}
				if ((c.y - MW::camera->y) * MW::camera->ay > 0) {
					dPy *= siny;

				}
			}
		}
	}
	if (collision) {
		MW::camera->colour = 0xff0000;
	} else {
		MW::camera->colour = 0xffffff;
	}
		
	// p = p + v*t + 1/2*a*t^2
	MW::camera->px = MW::camera->px + dPx;
	MW::camera->py = MW::camera->py + dPy;
	MW::camera->x = (uint32_t)(MW::camera->px + 0.5);
	MW::camera->y = (uint32_t)(MW::camera->py + 0.5);
	MW::camera->clampPosition(*MW::getDrawAreaPanel(Renderer::TOP_DOWN));

	MW::camera->direction = MW::camera->direction + dTheta;

	MW::camera->update();

	Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, CAMERA_STATUS);
	dbg.setMsg(&MW::event_message);
	dbg.setCamera(MW::camera);
	dbg.Print();
}

// Find the appropriate memory address that reflects the lower left point of the geometry object
void* MainWindow::findMemoryHandle(Geometry* g) {

	return MW::getRenderer()->getMemoryLocation(0, g->vertices.at(0));
}

void MainWindow::setWindowHeight(uint16_t p_height) {
	MW::window_height_ = p_height;
}

uint16_t MainWindow::getWindowHeight() {
	return MW::window_height_;
}

void MainWindow::setWindowWidth(uint16_t p_width) {
	MW::window_width_ = p_width;
}

uint16_t MainWindow::getWindowWidth() {
	return MW::window_width_;
}

void MainWindow::setWindowOffsetX(uint16_t p_offset) {
	MW::xPos_ = p_offset;
}

uint16_t MainWindow::getWindowOffsetX() {
	return MW::xPos_;
}

void MainWindow::setWindowOffsetY(uint16_t p_offset) {
	yPos_ = p_offset;
}

uint16_t MainWindow::getWindowOffsetY() {
	return MW::yPos_;
}

void MainWindow::setMTCOffsetX(uint8_t p_offset_x) {
	MW::main_to_client_offset_x_ = p_offset_x;
}

uint8_t MainWindow::getMTCOffsetX() {
	return MW::main_to_client_offset_x_;
}

void MainWindow::setMTCOffsetY(uint8_t p_offset_y) {
	MW::main_to_client_offset_y_ = p_offset_y;
}

uint8_t MainWindow::getMTCOffsetY() {
	return MW::main_to_client_offset_y_;
}

Rect& MainWindow::getMainWindowRect() {
	return main_rect;
}

void MainWindow::setMainWindowRect(HWND hwnd, Rect* rect) {

	RECT mwr;
	GetWindowRect(hwnd, &mwr);
	MW::main_rect = mwr;
}

Rect& MainWindow::getDrawRect() {
	
	return draw_rect;
}

void MainWindow::setDrawRect(HWND hwnd, Rect* rect) {

	RECT dr;
	GetClientRect(hwnd, &dr);
	MW::draw_rect = dr;
}

void MainWindow::conditionMouseCoords(Point2d& p) {

	// condition mouse cursor coordinates to be within range
	// 31 pixels is fixed for Y dimension due to title bar
	p.y -= (MW::getMainWindowRect().lt.y + 31);
	p.x -= (MW::getMainWindowRect().lt.x + MW::getMTCOffsetX());
}

void MainWindow::conditionMouseCoords(POINT& p) {

	// condition mouse cursor coordinates to be within range
	// 31 pixels is fixed for Y dimension due to title bar
	p.y -= (MW::getMainWindowRect().lt.y + 31);
	p.x -= (MW::getMainWindowRect().lt.x + MW::getMTCOffsetX());
}

bool MainWindow::canDrawHightlightLine() {
	return (GetKeyState(VK_LBUTTON) & 0x80) != 0 && MW::getRenderer()->getFocusLock() == 0 && MW::geo_start.isInitialized();
}
