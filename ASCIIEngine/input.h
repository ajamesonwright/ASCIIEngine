#include <cstdint>
#include <wtypes.h>
#include "Debug.h"
#include <vector>
#include "Geometry.h"
#define _USE_MATH_DEFINES
#include <math.h>
#ifndef ASCIIENGINE_INPUT_H_
#define ASCIIENGINE_INPUT_H_

enum Keys {
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

	ButtonState inputState[KEY_SIZE];

	void clearInput(bool clear_held = true, bool clear_update = true, int key = -1);
	void setInput(MSG* msg, bool is_held);
	bool getInput(WPARAM wp);
	bool getInput(int key_code);
	void handleInput(MSG* msg, float ms_per_frame);

private:
	Camera* camera;
	int vkToKey(WPARAM wp);
	void handleInput(int key_code);
};

#endif