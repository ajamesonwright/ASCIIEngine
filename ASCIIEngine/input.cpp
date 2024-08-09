#include "input.h"

void Input::clearInput(bool clear_held, bool clear_update, int key) {
	int range_begin = 0;
	int range_end = KEY_SIZE;
	if (key != -1) {
		range_begin = key;
		range_end = key + 1;
	}

	for (int i = range_begin; i < range_end; i++) {
		if (clear_held)
			inputState[i].held = false;
		if (clear_update)
			inputState[i].update = false;
	};
}

void Input::setInput(MSG* msg, bool is_held) {
	int code = vkToKey(msg->wParam);

	if (code) {
		Debug::DebugMessage dbg = Debug::DebugMessage(CallingClasses::INPUT_CLASS, DebugTypes::INPUT_DETECTED);
		dbg.setMsg(msg);
		Debug::Print(&dbg);

		inputState[code].update = is_held != inputState[code].held;
		inputState[code].held = is_held;
	}
}

bool Input::getInput(WPARAM wp) {
	int code = vkToKey(wp);

	if (code) {
		return inputState[code].held;
	}
	return false;
}

bool Input::getInput(int key_code) {
	return inputState[key_code].held;
}

void Input::handleInput(MSG* msg, float dt) {
	Debug::DebugMessage dbg = Debug::DebugMessage(CallingClasses::INPUT_CLASS, DebugTypes::INPUT_STATUS);
	Debug::Print(&dbg);

	double cosy = cos((camera->direction + 90) * M_PI / 180) * camera->moveSpeed;
	double sinx = sin((camera->direction + 90) * M_PI / 180) * camera->moveSpeed;

	if (inputState[W_DOWN].held) {
		camera->ay -= (float)cosy;
		camera->ax += (float)sinx;
	}
	if (inputState[S_DOWN].held) {
		camera->ay += (float)cosy;
		camera->ax -= (float)sinx;
	}
	if (inputState[A_DOWN].held) {
		camera->aa -= (float)camera->turnSpeed;
	}
	if (inputState[D_DOWN].held) {
		camera->aa += (float)camera->turnSpeed;
	}
	//camera->clampDirection();
}

int Input::vkToKey(WPARAM w_param) {
	uint32_t key_code = (uint32_t)w_param;
	switch (key_code) {
	case 0x57: // W
		return W_DOWN;
	case 0x41: // A
		return A_DOWN;
	case 0x53: // S
		return S_DOWN;
	case 0x44: // D
		return D_DOWN;
	case 0x80:
		return ML_DOWN;
	default:
		return 0;
	}
}