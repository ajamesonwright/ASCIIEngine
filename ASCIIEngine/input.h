#pragma once
class Input {
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

