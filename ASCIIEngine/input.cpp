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
			input_state[i].held = false;
		if (clear_update)
			input_state[i].update = false;
	};
}

void Input::setInput(MSG* msg, bool is_held) {
	int code = vkToKey(msg->wParam);

	if (code) {
		Debug::DebugMessage dbg = Debug::DebugMessage(CallingClasses::INPUT_CLASS, DebugTypes::INPUT_DETECTED);
		dbg.setMsg(msg);
		Debug::Print(&dbg);

		input_state[code].update = is_held != input_state[code].held;
		input_state[code].held = is_held;
	}
}

bool Input::getInput(WPARAM wp) {
	int code = vkToKey(wp);

	if (code)
		return input_state[code].held;
}

bool Input::getInput(int key_code) {
	return input_state[key_code].held;
}

void Input::handleInput(MSG* msg, float dt) {
	Debug::DebugMessage dbg = Debug::DebugMessage(CallingClasses::INPUT_CLASS, DebugTypes::INPUT_STATUS);
	Debug::Print(&dbg);
	if (input_state[A_DOWN].held) camera->direction -= (camera->turn_speed * dt);
	if (input_state[D_DOWN].held) camera->direction += (camera->turn_speed * dt);
	camera->clampDirection();

	double cosy = cos((camera->direction + 90) * M_PI / 180) * camera->move_speed;
	double sinx = sin((camera->direction + 90) * M_PI / 180) * camera->move_speed;

	if (input_state[W_DOWN].held) {
		camera->ay -= (float)cosy;
		camera->ax += (float)sinx;
	}
	if (input_state[S_DOWN].held) {
		camera->ay += (float)cosy;
		camera->ax -= (float)sinx;
	}
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