#include <iostream>
#include <string>
using namespace std;

string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

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

// 0-A, 1-B, ... 25-Z
int L_rotor_offset = 0;
int M_rotor_offset = 0;
int R_rotor_offset = 0;

void step(int &L, int &M, int &R) {

	R = (R + 1) % 26;

	// Consider notch turn over
	if (R == selected_rotor[3]->trigger - 'A') {

		M = (M + 1) % 26;

		if (M == selected_rotor[2]->trigger - 'A') {

			L = (L + 1) % 26;
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

	step(L_rotor_offset, M_rotor_offset, R_rotor_offset);

	int text_code = text - 'A';

	int rotor_R_result = rotor_forward_encode(text_code, RIGHT, R_rotor_offset);
	int rotor_M_result = rotor_forward_encode(rotor_R_result, MIDDLE, M_rotor_offset);
	int rotor_L_result = rotor_forward_encode(rotor_M_result, LEFT, L_rotor_offset);
	
	int reflector_result = reflector_encode(rotor_L_result, selected_reflector);

	int inv_rotor_L_result = rotor_inverse_encode(reflector_result, LEFT, L_rotor_offset);
	int inv_rotor_M_result = rotor_inverse_encode(inv_rotor_L_result, MIDDLE, M_rotor_offset);
	int inv_rotor_R_result = rotor_inverse_encode(inv_rotor_M_result, RIGHT, R_rotor_offset);

	//cout << "Rotor Position: " << char(L_rotor_offset + 'A') << char(M_rotor_offset + 'A') << char(R_rotor_offset + 'A') << endl;
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

typedef struct cycle {
	char input;
	int R_ind_array[10];
	int length;
} Cycle;

int KEY_PERMUTATION = 26 * 26 * 26;

int main() {
	selected_reflector = UKW_B;

	// Core initial indicator (Key)
	int core_L_ind;
	int core_M_ind;
	int core_R_ind;

	//				 123456789012
	// Plain Text:   DASXISTXEINX
	// Cripher Text: VJAREVEADJEV

	Cycle menu[6] = { {'S', {3,8,12,6}, 5}, {'S', {3,2,10,5,9,1,6}, 8} };
	int menu_length = 2;

	// Permute the rotors' order
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
				core_L_ind = 0;
				core_M_ind = 0;
				core_R_ind = 0;

				int count = 0;

				while (count < KEY_PERMUTATION) {

					for (int c = 0; c < 26; c++) {

						int menu_valid_count = 0;
						// Run for two menu case
						for (int i = 0; i < 2; i++) {

							char input = menu[i].input;

							if (alphabet[c] == input) continue;
							// Assume v1
							char v1 = alphabet[c];
							char vx = v1;
							// Set rotor indicators for case, then encode
							for (int x = 0; x < menu[i].length; x++) {
								L_rotor_offset = core_L_ind;
								M_rotor_offset = core_M_ind;
								R_rotor_offset = core_R_ind + menu[i].R_ind_array[x] - 1;
								vx = rotor_encode(vx);
							}

							if (vx == v1) {
								menu_valid_count++;
							}
						}

						// If all cycles are satisfied, print out possible settings of Enigma
						if (menu_valid_count == menu_length) {
							cout << "Possible Rotors: " << r_1 << ' ' << r_2 << ' ' << r_3 << endl;
							cout << "Possible Plugboard: " << menu[0].input << " <-> " << alphabet[c] << endl;
							cout << "Initial Core Setting: " <<
								char(core_L_ind + 'A') <<
								char(core_M_ind + 'A') <<
								char(core_R_ind + 'A')
								<< endl << endl;

							// Found key in this setting, no more cycles needed
							break;
						}
					}
					
					// Check if next key is valid
					step(core_L_ind, core_M_ind, core_R_ind);
					count++;
					
				}
			}
		}

	}

	return 0;
}