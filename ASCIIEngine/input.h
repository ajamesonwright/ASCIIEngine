#ifndef ASCIIENGINE_INPUT_H_
#define ASCIIENGINE_INPUT_H_

enum {
	ML_DOWN,
	MM_DOWN,
	MR_DOWN,

	KEY_SIZE,
};

class Input {

public:

	struct ButtonState {
		bool held = false;
		bool update = false;
	};

	ButtonState input_state[KEY_SIZE];

	void ClearInput() { for (int i = 0; i < KEY_SIZE; i++) { input_state[i].held = false; input_state[i].update = true; }; };
};

#endif