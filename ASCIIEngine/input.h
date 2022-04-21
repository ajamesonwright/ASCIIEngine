#ifndef ASCIIENGINE_INPUT_H_
#define ASCIIENGINE_INPUT_H_

class Input {

public:
	enum {
		ML_DOWN,
		MM_DOWN,
		MR_DOWN,

		KEY_SIZE,
	};

	struct ButtonState {
		bool held = false;
		bool update = false;
	};

	ButtonState input[KEY_SIZE];
};

#endif