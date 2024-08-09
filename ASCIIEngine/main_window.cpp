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
		Rect tempMwr = MW::getMainWindowRect();
		MW::setMainWindowRect(hwnd, &tempMwr);
		Rect tempDr = MW::getDrawRect();
		MW::setDrawRect(hwnd, &tempDr);

		if (MW::getRenderer()) {
			MW::renderer->setDrawArea(&MW::getDrawRect(), MW::borderWidth);
		}
		MW::getRenderer()->clearRenderArea(MW::renderer, true);
	} break;
	case WM_SIZE:
	{
		// Re-establish rect objects for both main window and draw area
		Rect& tempMwr = MW::getMainWindowRect();
		MW::setMainWindowRect(hwnd, &tempMwr);
		Rect& tempDr = MW::getDrawRect();
		MW::setDrawRect(hwnd, &tempDr);
		// Re-calculate offsets
		MW::setMTCOffsetX((tempMwr.getWidth()) - (tempDr.getWidth()) / 2);
		MW::setMTCOffsetY((tempMwr.getHeight() - (tempDr.getHeight()) - 31) / 2);

		// executed once per program run
		bool firstPass = false;
		if (!MW::getRenderer()) {
			MW::renderer = new Renderer(&tempDr, MW::borderWidth);
			firstPass = true;
		}

		if (MW::getRenderer() && !firstPass) {
			MW::renderer->setDrawArea(&tempDr, MW::borderWidth);
		}
		MW::getRenderer()->clearRenderArea(MW::renderer, true);
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
			//MW::setDrawMode(DrawMode::D_LINE);
		} break;
		case (0x54):
		{
			// 'T'
			//MW::setDrawMode(DrawMode::D_TRI);
		} break;
		case (0x52):
		{
			// 'R'
			//MW::setDrawMode(DrawMode::D_RECT);
		} break;
		case (0x51):
		{
			// 'Q'
			//MW::setDrawMode(DrawMode::D_QUAD);
		} break;
		case (0x43):
		{
			// 'C'
			//MW::setDrawMode(DrawMode::D_CIRCLE);
		} break;
		case (VK_ESCAPE):
		{
			MW::input->clearInput();
			// clear focus lock or exit if no lock present
			int locked = MW::getRenderer()->getFocusLock();
			if (locked != -1) {
				Debug::DebugMessage dbg(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::PANEL_LOCK);
				dbg.setMsg(&MW::eventMessage);
				dbg.setPanelId(MW::currentPanel);
				dbg.setLockedPanel(locked);
				Debug::Print(&dbg);

				MW::getRenderer()->setFocusLock(-1);
				MW::getRenderer()->updateRenderArea(MW::getRenderer()->getDrawArea()->panels[locked], locked, MW::getRenderer()->colours[locked][MW::currentPanel == locked ? 1 : 0], true);
				break;
			}
			MW::setRunningState(STOPPED);
		} break;
		}
	} break;
	case WM_MOUSEMOVE:
	{
		Debug::DebugMessage dbg(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::MOUSE_POSITION);
		dbg.setMsg(&MW::eventMessage);
		dbg.setPanelId(MW::currentPanel);
		dbg.Print();

		// Verify that the cursor has changed panels since last update and no panel is currently locked
		if (MW::currentPanel == MW::renderer->getFocus() || MW::renderer->getFocusLock() != -1)
			break;
		MW::renderer->setFocus(MW::currentPanel);

		// Reset TOP_DOWN panel
		if (MW::currentPanel != Renderer::TOP_DOWN) {
			MW::renderer->updateRenderArea(*MW::getDrawAreaPanel(Renderer::TOP_DOWN), Renderer::TOP_DOWN, MW::renderer->colours[Renderer::TOP_DOWN][0], true);
		}
		// Reset FIRST_PERSON panel
		if (MW::currentPanel != Renderer::FIRST_PERSON) {
			MW::renderer->updateRenderArea(*MW::getDrawAreaPanel(Renderer::FIRST_PERSON), Renderer::FIRST_PERSON, MW::renderer->colours[Renderer::FIRST_PERSON][0], true);
		}
		if (MW::currentPanel == Renderer::BACKGROUND)
			break;

		MW::renderer->updateRenderArea(MW::renderer->getDrawArea()->panels[MW::currentPanel], MW::currentPanel, MW::renderer->colours[MW::currentPanel][1], true);
	} break;
	case WM_LBUTTONDOWN:
	{
		bool insideExistingGeometry = false;
		Point2d start = MW::eventMessage.pt;
		for (const auto g : MW::geometryQueue) {
			if (g->checkCollisionWith(start)) {
				// Disallow starting new geometry when starting from insde existing geometry
				insideExistingGeometry = true;
			}
		}
		{
			Debug::DebugMessage dbg(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::INPUT_DETECTED);
			dbg.setMsg(&MW::eventMessage);
			dbg.setPanelId(MW::currentPanel);
			dbg.Print();
		}

		if (insideExistingGeometry) {
			break;
		}
		if (MW::getRenderer()->getFocusLock() == -1 && MW::currentPanel != Renderer::BACKGROUND) {
			MW::getRenderer()->setFocusLock(MW::currentPanel);
			Debug::DebugMessage dbg(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::PANEL_LOCK);
			dbg.setMsg(&MW::eventMessage);
			dbg.setPanelId(MW::currentPanel);
			dbg.Print();
			break;
		}

		if (MW::getRenderer()->getFocusLock() != -1 && MW::currentPanel == Renderer::TOP_DOWN) {
			MW::geoStart = MW::eventMessage.pt;
			MW::input->inputState[ML_DOWN].held = true;
			MW::input->inputState[ML_DOWN].update = true;
		}
	} break;
	case WM_LBUTTONUP:
	{
		{
			Debug::DebugMessage dbg(CallingClasses::MAIN_WINDOW_CLASS, DebugTypes::INPUT_DETECTED);
			dbg.setMsg(&MW::eventMessage);
			dbg.setPanelId(MW::currentPanel);
			dbg.Print();
		}

		if (MW::input->getInput(ML_DOWN)) {
			// Clear held for each button up message if it qualified as 'click and drag' action
			MW::input->inputState[ML_DOWN].held = false;
			MW::input->inputState[ML_DOWN].update = true;
		} else
			break;

		// Check for top-down focus lock before calculating square end point
		if (MW::getRenderer()->getFocusLock() == 0 && !MW::input->inputState[ML_DOWN].held) {
			MW::geoEnd = MW::eventMessage.pt;

			if (MW::geoStart.displacementFrom(MW::geoEnd) > 10) {
				Geometry* g;

				Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, GEO_QUEUE_MOD);
				dbg.setMsg(&MW::eventMessage);
				dbg.setPanelId(MW::currentPanel);
				dbg.setDrawMode(MW::drawMode);

				switch (MW::drawMode) {
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
				dbg.setGeometry(g);
				dbg.Print();

				MW::addGeometry(g);
			}
		}
		MW::geoStart = Point2d();
		MW::geoEnd = Point2d();
	} break;
	case WM_RBUTTONUP:
	{
		if (MW::geometryQueue.size() > 0 && MW::getRenderer()->getFocusLock() == Renderer::TOP_DOWN) {
			Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, GEO_QUEUE_MOD);
			dbg.setMsg(&MW::eventMessage);
			dbg.setPanelId(MW::currentPanel);
			dbg.setDrawMode(0);
			dbg.setGeometry(MW::geometryQueue.back());
			dbg.Print();

			MW::removeGeometry(MW::geometryQueue.back());
			MW::getRenderer()->getDrawArea()->update = true;
		}
		MW::getRenderer()->clearRenderArea(MW::renderer);
	} break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
return 0;
}

static void cleanUp() {
	MW::getRenderer()->cleanUp();
	delete MW::getRenderer();
	delete MW::input;
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
	//AdjustWindowRect(&mainRect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, true);
	hwnd = CreateWindow(wc.lpszClassName, L"ASCII Engine", WS_OVERLAPPEDWINDOW | WS_VISIBLE, MW::windowStartingX, MW::windowStartingY, MW::windowStartingWidth, MW::windowStartingHeight, 0, 0, hInstance, 0);

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
		MW::setMTCOffsetX(((MW::mainRect.getWidth()) - (MW::drawRect.getWidth())) / 2);
		MW::setMTCOffsetY(((MW::mainRect.getHeight()) - (MW::drawRect.getHeight()) - 31) / 2);
	}


	// Clear DrawArea
	MW::getRenderer()->clearRenderArea(MW::renderer, true);
	// set initial viewport location and orientation
	{
		Rect* topDownPanel = MW::getDrawAreaPanel(Renderer::TOP_DOWN);
		MW::camera = new Camera(topDownPanel->lt.x + topDownPanel->getWidth() / 2, topDownPanel->lt.y + topDownPanel->getHeight() / 2, -90);
		MW::camera->setSize(20);
	    
	    // set initial inputs
		MW::input = new Input(MW::camera);
	}
	// Initialize renderer threads
	MW::renderer->init(*MW::camera);

	// Create base quadtree
	MW::qt = new Quadtree(*MW::getDrawAreaPanel(Renderer::TOP_DOWN));

	// Establish framerate metrics
	const float TARGET_FRAMERATE = 60.0f;
	float dt = 1.0f / TARGET_FRAMERATE;
	LARGE_INTEGER frameBeginTime, frameEndTime;
	QueryPerformanceCounter(&frameBeginTime);
	float frameUpdateFrequency;
	{
		LARGE_INTEGER performanceFrequencyMeasure;
		QueryPerformanceFrequency(&performanceFrequencyMeasure);
		frameUpdateFrequency = (float)performanceFrequencyMeasure.QuadPart;
	}

	// Program loop
	while (MW::getRunningState()) {
		// Reset inputs
		MW::input->clearInput(0, 1);
		MW::getRenderer()->clearRenderArea(MW::renderer);
		// Main message loop
		while (PeekMessage(&MW::eventMessage, hwnd, 0, 0, PM_REMOVE)) {
			// Update panel under mouseover for highlight
			MW::conditionMouseCoords(MW::eventMessage.pt);
			MW::currentPanel = MW::getCursorFocus((Point2d)MW::eventMessage.pt);

			bool isHeld = ((MW::eventMessage.lParam & (1U << 31)) == 0);
			MW::input->setInput(&MW::eventMessage, isHeld);

			TranslateMessage(&MW::eventMessage);
			DispatchMessage(&MW::eventMessage);
		}

		// Perform actions for updated input_
		MW::input->handleInput(&MW::eventMessage, dt);
		MW::simulateFrame(dt);

		MW::renderer->clearRenderArea(MW::renderer, true);
		// Draw highlight line for click and hold
		if (MW::canDrawHightlightLine()) {
			MW::drawHighlightLine();
		}

		// Draw camera
		MW::renderer->updateRenderArea(*MW::camera, Renderer::TOP_DOWN, MW::camera->colour, false);
		// Draw geometry queue from oldest to newest
		for (int i = 0; i < MW::geometryQueue.size(); i++) {
			MW::renderer->updateRenderArea(MW::geometryQueue[i], Renderer::TOP_DOWN);
		}
		// Draw quadtree grid
		std::vector<Line*>* grid = MW::qt->getQuadtreeGrid();
		for (int i = 0; i < grid->size(); i++) {
			MW::getRenderer()->updateRenderArea(grid->at(i), Renderer::TOP_DOWN, 0xff0000, true);
		}

		// Pass geometry queue and camera to the renderer to determine how to update the buffer
		MW::renderer->updateRenderArea(MW::geometryQueue, *MW::camera);

		// Draw updated RenderArea to screen
		MW::renderer->drawRenderArea(hdc);

		QueryPerformanceCounter(&frameEndTime);
		dt = (float)(frameEndTime.QuadPart - frameBeginTime.QuadPart) / frameUpdateFrequency;

		MW::camera->turnSpeed = 14000 + (150 - 14000) * (dt - 1.0 / 300) / (1.0 / 35 - 1.0 / 300);
		MW::camera->moveSpeed = 5000 + (600 - 5000) * (dt - 1.0 / 300) / (1.0 / 35 - 1.0 / 300);
		Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, FRAMES_PER_SECOND);
		dbg.setMsg(&MW::eventMessage);
		dbg.setFps(dt);
		Debug::Print(&dbg);
		frameBeginTime = frameEndTime;
	}

	cleanUp();
	return 0;
}

Renderer* MainWindow::getRenderer() {
	return renderer;
}

Rect* MainWindow::getDrawAreaPanel(int panel) {
	return &MW::renderer->getDrawArea()->panels[panel];
}

bool MainWindow::getRunningState() {
	return MW::runState;
}

void MainWindow::setRunningState(int runState) {
	if (runState == MW::runState)
		return;
	if (runState != 0 && runState != 1)
		return;

	MW::runState = runState;
}

void MainWindow::setDrawMode(int drawMode) {

	if (drawMode == MW::drawMode)
		return;
	if (drawMode < 0 || drawMode >= D_DRAW_MODE_SIZE)
		return;

	Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, DRAW_MODE_CHANGED);
	dbg.setMsg(&MW::eventMessage);
	dbg.setDrawMode(drawMode);
	Debug::Print(&dbg);
	MW::drawMode = drawMode;
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

	MW::geometryQueue.push_back(g);
	MW::qt->addGeometry(g);
}

void MainWindow::removeGeometry(Geometry* g) {

	MW::geometryQueue.pop_back();
	MW::qt->removeGeometry(g);
}

void MainWindow::simulateFrame(float dt) {

	// Damping effect on acceleration
	MW::camera->ax = MW::camera->ax * MW::camera->accelerationDamping;
	MW::camera->ay = MW::camera->ay * MW::camera->accelerationDamping;
	//MW::camera->clampAcceleration();
	// v = v + a*t
	MW::camera->vx = (MW::camera->vx + MW::camera->ax * dt) * MW::camera->velocityDamping;
	MW::camera->vy = (MW::camera->vy + MW::camera->ay * dt) * MW::camera->velocityDamping;
	//MW::camera->clampVelocity();
	// Incremental change in position
	double dPx = MW::camera->vx * dt + MW::camera->ax * dt * dt * 0.5f;
	double dPy = MW::camera->vy * dt + MW::camera->ay * dt * dt * 0.5f;

	// Angular acceleration
	MW::camera->aa = MW::camera->aa * MW::camera->accelerationDamping;
	//MW::camera->clampAngularAcceleration();
	// Angular velocity
	MW::camera->va = (MW::camera->va + MW::camera->aa * dt) * MW::camera->velocityDamping;
	// Incremental change in direction
	double dTheta = MW::camera->va * dt;

	bool collision = false;
	Point2d center = Point2d(MW::camera->x, MW::camera->y);
	Line xAxis = Line(Point2d(0, 0), Point2d(1, 0));
	for (int i = 0; i < MW::geometryQueue.size(); i++) {
		std::map<Point2d, std::vector<Line>> collisions;
		MW::camera->checkCollisionWith(geometryQueue[i], collisions);

		if (!collisions.empty()) {
			collision = true;
			// Determine normal of each collision and adjust position of camera to remove collision
			int count = 0;
			for (auto pair : collisions) {
				count++;
				Point2d c = pair.first;
				Line normal = Line(c, center);
				for (auto side : pair.second) {
					Line testLine = Line(side.vertices.at(1), side.vertices.at(0));
					if (abs(normal.calculateAngle(side) * 180 / M_PI - 90) > 3) {
						continue;
					}

					double angleOfXAxisToSide = xAxis.calculateAngle(side);
					double alpha = angleOfXAxisToSide > 90 ? (180 - angleOfXAxisToSide) : angleOfXAxisToSide;
					// Calculate magnitude of current change in position (analogous to speed)
					double totalVelocity = sqrt(dPx * dPx + dPy * dPy);
					double angleOfMotionToSide = acos((dPx * side.getDx() + dPy * side.getDy()) / (totalVelocity * side.calculateLength()));
					double gamma = angleOfMotionToSide > 90 ? (180 - angleOfMotionToSide) : angleOfMotionToSide;
					// Calculate the new velocity produced by sliding along the side of the geometry
					double newVelocity = totalVelocity * cos(gamma);

					// Calculate the x and y components of that 'velocity'
					if (normal.getDx() * MW::camera->ax < 0) {
						dPx = newVelocity * cos(alpha);
					}
					if (normal.getDy() *MW::camera->ay < 0) {
						dPy = newVelocity * sin(alpha);
					}
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
	MW::camera->clampDirection();

	MW::camera->update();

	Debug::DebugMessage dbg(MAIN_WINDOW_CLASS, CAMERA_STATUS);
	dbg.setMsg(&MW::eventMessage);
	dbg.setCamera(MW::camera);
	dbg.Print();
}

// Find the appropriate memory address that reflects the lower left point of the geometry object
void* MainWindow::findMemoryHandle(Geometry* g) {

	return MW::getRenderer()->getMemoryLocation(0, g->vertices.at(0));
}

void MainWindow::setWindowHeight(uint16_t height) {
	MW::windowHeight = height;
}

uint16_t MainWindow::getWindowHeight() {
	return MW::windowHeight;
}

void MainWindow::setWindowWidth(uint16_t width) {
	MW::windowWidth = width;
}

uint16_t MainWindow::getWindowWidth() {
	return MW::windowWidth;
}

void MainWindow::setWindowOffsetX(uint16_t offset) {
	MW::xPos = offset;
}

uint16_t MainWindow::getWindowOffsetX() {
	return MW::xPos;
}

void MainWindow::setWindowOffsetY(uint16_t offset) {
	yPos = offset;
}

uint16_t MainWindow::getWindowOffsetY() {
	return MW::yPos;
}

void MainWindow::setMTCOffsetX(uint8_t offset) {
	MW::mainToClientOffsetX = offset;
}

uint8_t MainWindow::getMTCOffsetX() {
	return MW::mainToClientOffsetX;
}

void MainWindow::setMTCOffsetY(uint8_t offset) {
	MW::mainToClientOffsetY = offset;
}

uint8_t MainWindow::getMTCOffsetY() {
	return MW::mainToClientOffsetY;
}

Rect& MainWindow::getMainWindowRect() {
	return mainRect;
}

void MainWindow::setMainWindowRect(HWND hwnd, Rect* rect) {

	RECT mwr;
	GetWindowRect(hwnd, &mwr);
	MW::mainRect = mwr;
}

Rect& MainWindow::getDrawRect() {
	
	return drawRect;
}

void MainWindow::setDrawRect(HWND hwnd, Rect* rect) {

	RECT dr;
	GetClientRect(hwnd, &dr);
	MW::drawRect = dr;
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
	return (GetKeyState(VK_LBUTTON) & 0x80) != 0 && MW::getRenderer()->getFocusLock() == 0 && MW::geoStart.isInitialized();
}

void MainWindow::drawHighlightLine() {
	// Cast to custom point type and clamp to window dimensions
	Point2d end = MW::eventMessage.pt;
	MW::renderer->clampDimension(end.x, Renderer::Dimension::HORIZONTAL, Renderer::BACKGROUND);
	MW::renderer->clampDimension(end.y, Renderer::Dimension::VERTICAL, Renderer::BACKGROUND);
	MW::highlightLine = Line(MW::geoStart, end);

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
			Point2d start = MW::geoStart;
			std::sort(collisions.begin(), collisions.end(), [&start](Point2d& p1, Point2d& p2) {
				return p1.displacementFrom(start) < p2.displacementFrom(start);
				});
		}
		// Clip to the geometry closest to start of line
		MW::highlightLine = Line(MW::geoStart, Point2d(collisions.at(0)));
	}
	MW::renderer->updateRenderArea(MW::highlightLine, Renderer::TOP_DOWN, colour, false);

	if (MW::outlineType) {
		MW::renderer->updateRenderArea(Line(MW::geoStart, Point2d(MW::geoStart.x, end.y)), Renderer::TOP_DOWN, 0xff00, false);
		MW::renderer->updateRenderArea(Line(MW::geoStart, Point2d(end.x, MW::geoStart.y)), Renderer::TOP_DOWN, 0xff00, false);

		MW::renderer->updateRenderArea(Line(Point2d(MW::geoStart.x, end.y), end), Renderer::TOP_DOWN, 0xff00, false);
		MW::renderer->updateRenderArea(Line(Point2d(end.x, MW::geoStart.y), end), Renderer::TOP_DOWN, 0xff00, false);
	}
}
