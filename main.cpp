#include <iostream>
#include <chrono>

using namespace std;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::seconds;

// 0-A, 1-B, ... 25-Z
char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

enum position
{
	LEFT = 1,
	MIDDLE = 2,
	RIGHT = 3
};

typedef struct rotor {
	int id;
	char trigger;
} Rotor;

const char* rotor_wiring[11] = {
	"", // Dummy
  // ABCDEFGHIJKLMNOPQRSTUVWXYZ
	"EKMFLGDQVZNTOWYHXUSPAIBRCJ", // Rotor 1
	"AJDKSIRUXBLHWTMCQGZNPYFVOE", // Rotor 2
	"BDFHJLCPRTXVZNYEIWGAKMUSQO", // Rotor 3
	"ESOVPZJAYQUIRHXLNFTGKDCMWB", // Rotor 4
	"VZBRGITYUPSDNHLXAWMJQOFECK",  // Rotor 5
  // Inverse
  // ABCDEFGHIJKLMNOPQRSTUVWXYZ
	"UWYGADFPVZBECKMTHXSLRINQOJ", // Inv Rotor 1
	"AJPCZWRLFBDKOTYUQGENHXMIVS", // Inv Rotor 2
	"TAGBPCSDQEUFVNZHYIXJWLRKOM", // Inv Rotor 3
	"HZWVARTNLGUPXQCEJMBSKDYOIF", // Inv Rotor 4
	"QCYLXWENFTZOSMVJUDKGIARPHB"  // INv Rotor 5
};

const char *reflector_wiring[2] = {
//	 ABCDEFGHIJKLMNOPQRSTUVWXYZ
	"YRUHQSLDPXNGOKMIEBFZCWVJAT", // Reflector UKW-B
	"FVPJIAOYEDRZXWGCTKUQSBNMHL"  // Reflector UKW-C
};

Rotor rotor_choices[6] = {
	{0, 'X'}, // Dummmy 
	{1, 'R'}, {2, 'F'}, {3, 'W'}, {4, 'K'}, {5, 'A'}
};

// 0 - Dummy, 1 - Left , 2 - Middle, 3 - Right
Rotor* selected_rotor[4];

enum reflector_type
{
	UKW_B = 0,
	UKW_C = 1
};
// 0 - UKW_B, 1 - UKW_C
reflector_type selected_reflector = UKW_B;

// Ring setting is fixed at AAA

// Indicator value 0 ~ 25
int rotor_indicator[4] = {
	-1, // Dummy
	0,	// Left
	0,	// Middle
	0	// Right
};

void step(int* rotor_indicator) {

	rotor_indicator[RIGHT] = (rotor_indicator[RIGHT] + 1) % 26;

	// Consider notch turn over
	if (rotor_indicator[RIGHT] == selected_rotor[3]->trigger - 'A') {

		rotor_indicator[MIDDLE] = (rotor_indicator[MIDDLE] + 1) % 26;

		if (rotor_indicator[MIDDLE] == selected_rotor[2]->trigger - 'A') {

			rotor_indicator[LEFT] = (rotor_indicator[LEFT] + 1) % 26;
		}
	}
}

// A -> Z, B -> A
int backward_offset(int index, int offset) {
	int result = index - offset;

	if (result < 0) {
		result = 26 + result;
	}

	return result;
}

int rotor_forward_encode(int text_code, position pos, int rotor_offset) {
	int temp = rotor_wiring[selected_rotor[pos]->id][(text_code + rotor_offset) % 26] - 'A';

	return alphabet[backward_offset(temp, rotor_offset)] - 'A';
}

int rotor_inverse_encode(int text_code, position pos, int rotor_offset) {
	int temp = alphabet[(text_code + rotor_offset) % 26] - 'A';

	temp = rotor_wiring[selected_rotor[pos]->id + 5][temp] - 'A';

	return alphabet[backward_offset(temp, rotor_offset)] - 'A';
}

int reflector_encode(int text_code, reflector_type type) {
	return reflector_wiring[type][text_code] - 'A';
}

char rotor_encode(char text) {

	step(rotor_indicator);

	int text_code = text - 'A';

	int rotor_R_result = rotor_forward_encode(text_code, RIGHT, rotor_indicator[RIGHT]);
	int rotor_M_result = rotor_forward_encode(rotor_R_result, MIDDLE, rotor_indicator[MIDDLE]);
	int rotor_L_result = rotor_forward_encode(rotor_M_result, LEFT, rotor_indicator[LEFT]);
	
	int reflector_result = reflector_encode(rotor_L_result, selected_reflector);

	int inv_rotor_L_result = rotor_inverse_encode(reflector_result, LEFT, rotor_indicator[LEFT]);
	int inv_rotor_M_result = rotor_inverse_encode(inv_rotor_L_result, MIDDLE, rotor_indicator[MIDDLE]);
	int inv_rotor_R_result = rotor_inverse_encode(inv_rotor_M_result, RIGHT, rotor_indicator[RIGHT]);

	// Print each step
	//cout << "Rotor Position: " << char(rotor_indicator[LEFT] + 'A') 
	//	<< char(rotor_indicator[MIDDLE] + 'A') << char(rotor_indicator[RIGHT] + 'A') << endl;
	//cout << "Rotor R Encrytion: " << char(rotor_R_result + 'A') << endl;
	//cout << "Rotor M Encrytion: " << char(rotor_M_result + 'A') << endl;
	//cout << "Rotor L Encrytion: " << char(rotor_L_result + 'A') << endl;
	//cout << "Reflector Encrytion: " << char(reflector_result + 'A') << endl;
	//cout << "Inv Rotor L Encrytion: " << char(inv_rotor_L_result + 'A') << endl;
	//cout << "Inv Rotor M Encrytion: " << char(inv_rotor_M_result + 'A') << endl;
	//cout << "Inv Rotor R Encrytion: " << char(inv_rotor_R_result + 'A') << endl << endl;
	//cout << char(inv_rotor_R_result + 'A');

	return inv_rotor_R_result + 'A';
}

typedef struct loop {
	const char* text;
	int index[10];
	int length;
} Loop;

const int KEY_PERMUTATION_COUNT = 26 * 26 * 26;

void print_plug_board(int* plugboard) {

	for (int c = 0; c < 26; c++) {
		cout << char(c + 'A');

		if (plugboard[c] != -1) {
			cout << char(plugboard[c] + 'A');
		}
		else {
			cout << '?';
		}

		cout << ' ';
	}
	cout << endl;
}

void reset_plugboard(int* plugboard) {
	for (int i = 0; i < 26; i++) {
		plugboard[i] = -1;
	}
}

bool is_loop_satisfied(Loop* loop, int* plugboard, int start_index, int* core_rotor_indicator) {
	char input = loop->text[start_index];
	char v1;

	if (plugboard[input - 'A'] == -1) {
		v1 = input;
	}
	else {
		v1 = char(plugboard[input - 'A'] + 'A');
	}
	

	if (v1 < 'A' || v1 > 'Z') {
		cout << "v1 error: " << v1 << endl;
		return false;
	}

	char vx = v1;

	// Set rotor indicators for the case, then encode
	for (int i = 0; i < loop->length; i++) {
		rotor_indicator[LEFT] = core_rotor_indicator[LEFT];
		rotor_indicator[MIDDLE] = core_rotor_indicator[MIDDLE];
		rotor_indicator[RIGHT] = core_rotor_indicator[RIGHT];

		// Set to the right indicator
		for (int i = 0; i < loop->index[start_index]; i++) {
			step(rotor_indicator);
		}

		vx = rotor_encode(vx);
		start_index = (start_index + 1) % loop->length;
	}

	if (vx == v1) {
		return true;
	}
	else {
		return false;
	}
}

bool dfs_check_loop(int* plugboard, int plugboard_count, int loop_index, Loop* loop, int* core_rotor_indicator) {
	if (loop_index == loop->length) {
		return true;
	}

	// First char in loop
	int input = loop->text[loop_index] - 'A';

	// If previous plugboard setting exists or plugboard is full
	// Just check if loop is satisfied
	if (plugboard[input] != -1 || plugboard_count == 10) {
		if (is_loop_satisfied(loop, plugboard, loop_index, core_rotor_indicator)) {
			if (dfs_check_loop(plugboard, plugboard_count, loop_index + 1, loop, core_rotor_indicator)) {
				return true;
			}
		}
	}
	// Else guess the new plugboard setting
	else {
		for (int c = 0; c < 26; c++) {
			// If there is no contradiction
			if (plugboard[c] == -1) {
				plugboard[input] = c;
				plugboard[c] = input;

				if (is_loop_satisfied(loop, plugboard, loop_index, core_rotor_indicator)) {
					if (dfs_check_loop(plugboard, plugboard_count + 1,loop_index + 1, loop, core_rotor_indicator)) {
						return true;
					}
				}

				// Loop not satisfied
				// Backtrack
				plugboard[input] = -1;
				plugboard[c] = -1;
			}
		}
	}

	return false;
}

int main() {
	
	// Core initial indicator (Key)
	int core_rotor_indicator[4] = {
		-1, // Dummy
		0,	// Left
		0,	// Middle
		0	// Right
	};

	selected_reflector = UKW_B;

	//				 12345678901234567890123
	// Plain Text:   WETTERVORHERSAGEBISKAYA
	// Cripher Text: RWIVTYRESXBFOGKUHQBAISE

	// -1 = no swap
	int plugboard[26];

	Loop loop[4] = { {"AGK", {14,15,20}, 3}, {"AITE", {21,3,5,23}, 4}, {"EWRVT", {2,1,7,4,5}, 5}, {"EBSRW", {11,19,9,1,2}, 5} };
	int loop_count = 4;

	int possible_solution_count = 0;

	auto start_time = high_resolution_clock::now();

	// Start the bombe machine
	// Permute rotors' order
	for (int r_1 = 1; r_1 <= 5; r_1++) {
		for (int r_2 = 1; r_2 <= 5; r_2++) {
			if (r_1 == r_2) continue;
			for (int r_3 = 1; r_3 <= 5; r_3++) {
				if (r_3 == r_2 || r_3 == r_1) continue;

				selected_rotor[1] = &rotor_choices[r_1];
				selected_rotor[2] = &rotor_choices[r_2];
				selected_rotor[3] = &rotor_choices[r_3];

				// Assume Ring Settings is AAA
				// Start from AAA key
				core_rotor_indicator[LEFT] = 0;
				core_rotor_indicator[MIDDLE] = 0;
				core_rotor_indicator[RIGHT] = 0;

				int key_count = 0;

				// Iterate through keys
				while (key_count < KEY_PERMUTATION_COUNT) {

					int satisfied_loop_count = 0;

					reset_plugboard(plugboard);

					for (int n = 0; n < loop_count; n++) {

						if (dfs_check_loop(plugboard, 0, 0, &(loop[n]), core_rotor_indicator)) {
							satisfied_loop_count++;
						}

					}

					// Print settings if count = 3
					// Fixed to 3 here, cuz if 4 no solution can be found
					if (satisfied_loop_count == 3) {
						possible_solution_count++;

						cout << possible_solution_count << endl;
						cout << "Possible Rotors: " << r_1 << ' ' << r_2 << ' ' << r_3 << endl;
						cout << "Possible Plugboard: "; print_plug_board(plugboard);
						cout << "Initial Core Setting: " <<
							char(core_rotor_indicator[LEFT] + 'A') <<
							char(core_rotor_indicator[MIDDLE] + 'A') <<
							char(core_rotor_indicator[RIGHT] + 'A')
							<< endl << endl;
					}

					// Check if next key is valid
					step(core_rotor_indicator);
					key_count++;
					
				}
			}
		}

	}

	auto end_time = high_resolution_clock::now();

	auto s_int = duration_cast<seconds>(end_time - start_time);
	std::cout << "Time Elapsed: " << s_int.count() << "s" << endl;

	return 0;
}