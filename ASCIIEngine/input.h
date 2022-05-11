#include <cstdint>
#include <wtypes.h>
#include "debug.h"
#include <vector>
#include "geometry.h"
#define _USE_MATH_DEFINES
#include <math.h>
#ifndef ASCIIENGINE_INPUT_H_
#define ASCIIENGINE_INPUT_H_

enum keys {
	ML_DOWN,
	W_DOWN,
	A_DOWN,
	S_DOWN,
	D_DOWN,

	KEY_SIZE,
};

class Input {

	struct ButtonState {
		bool held = false;
		bool update = false;
	};

public:
	Input(Camera* p_camera) {
		camera = p_camera;
	}

	ButtonState input_state[KEY_SIZE];

	void ClearInput(bool clear_held = true, bool clear_update = true, int key = -1);
	void SetInput(MSG* msg, bool is_held);
	bool GetInput(WPARAM wp);
	bool GetInput(int key_code);
	void HandleInput(MSG* msg, float ms_per_frame);

private:
	Camera* camera;
	int VkToKey(WPARAM wp);
	void HandleInput(int key_code);
};

#endif