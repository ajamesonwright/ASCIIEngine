#ifndef ASCIIENGINE_INPUT_H_
#define ASCIIENGINE_INPUT_H_

class Input {

public:
	enum {
		ML_DOWN,
		MM_DOWN,
		MR_DOWN,
		KEY_DOWN,

		KEY_SIZE,
	};

	struct Input_State {
		bool held = false;
		bool update = false;
	};

	struct Inputs {
		Input_State input[KEY_SIZE];
	} input_;
};

#endif