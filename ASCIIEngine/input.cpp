#include "input.h"

void Input::ClearInput(bool clear_held, bool clear_update, int key) {
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

void Input::SetInput(MSG* msg, bool is_held) {
	int code = VkToKey(msg->wParam);

	if (code) {
		debug::PrintDebugMsg(calling_class::INPUT_CLASS, debug_type::INPUT_DETECTED, msg, -1, -1, nullptr, -1);
		input_state[code].update = is_held != input_state[code].held;
		input_state[code].held = is_held;
	}
}

bool Input::GetInput(WPARAM wp) {
	int code = VkToKey(wp);

	if (code)
		return input_state[code].held;
}

bool Input::GetInput(int key_code) {
	return input_state[key_code].held;
}

void Input::HandleInput(float s_per_frame) {
	if (input_state[W_DOWN].held) camera->ay = camera->ay - 1;
	if (input_state[S_DOWN].held) camera->ay = camera->ay + 1;

	if (input_state[A_DOWN].held) camera->direction = camera->direction - 1;
	if (input_state[D_DOWN].held) camera->direction = camera->direction + 1;
}

int Input::VkToKey(WPARAM w_param) {
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
	default:
		return 0;
	}
}